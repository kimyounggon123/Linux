#include "SocketIPC.hpp"

bool SocketIPC::MakeSocket(const char* addr)
{   
    switch (protocol)
    {
        case ProtocolType::TCP:
            sock = socket(domain, SOCK_STREAM, IPPROTO_TCP);
            break;
        case ProtocolType::UDP:
            sock = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
            break;        
        case ProtocolType::UNIX:
            domain = AF_UNIX;
            sock = socket(domain, SOCK_STREAM, 0);
            break;
        default:
            sock = -1;
            return false;
    }
    
    // recv timeout
    timeval tv{};
    tv.tv_sec = 1; // n sec waiting
    tv.tv_usec = 0; // 100000 us = 100 ms
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = domain;
    serverAddr.sin_port = htons(port);
    int result = inet_pton(domain, addr, &serverAddr.sin_addr);    
    if (result == -1)
    {
        std::cout << "inet_pton" << std::endl;
        return false;
    }
    if (protocol == ProtocolType::TCP)
    {
        bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
        result = connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == -1)
        {
            std::cerr << "connect error: "
                << std::error_code(errno, std::generic_category()).message()
                << std::endl;
            return false;
        }
    }

    return true;
}

void SocketIPC::Start() 
{
    recvPool.Start("ICP Recver");
    sendPool.Start("ICP Sender");
}

void TCP_IPC::Recver::Work()
{
    while (isRunning)
    {
        if (buffer.GetVoidSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
        int retval = recv(sock, buffer.GetBufferToRead(), buffer.GetVoidSpace(), 0);
        if (retval == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // timeout
                continue;
            }
            else
            {
                isRunning = false;
            }
        }   
        buffer.OnWrite(retval);
        MakePacketFromBuffer();
    }
}

void TCP_IPC::Recver::MakePacketFromBuffer()
{
    Packet* pk = nullptr;

    while (!buffer.IsEmpty())
    {
        pk = pkPool->Acquire();
        if (pk == nullptr) 
        {
            LogTool::Log("Reader Work", "packet is null");
            break;
        }
        // 현재 버퍼에 남아있는 데이터의 총 크기 계산
        // 2. 역직렬화 시도
        pk->ClearBuffer();
        int rcvLen = buffer.GetCurrDataSize();
        ERROR_CODE code = pk->Deserialize(buffer.GetReadPtr(), rcvLen);
            
        // 3. 역직렬화 실패 처리
        if (code != ERROR_CODE::SUCCESS) // 또는 !code (SUCCESS가 0인 경우)
        {
            std::cout << "Deserialize fail" << std::endl;
            if (code == ERROR_CODE::NEED_EXTRA_DATA)
            {
                std::cout << "NEED_EXTRA_DATA" << std::endl;
                // 데이터가 더 필요하므로, 남은 데이터를 버퍼 앞쪽으로 당기고 다음 recv를 기다림
                buffer.MoveDataFront();
                pkPool->Release(pk);
                break;
            }

            else
            {
                std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
                // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
                // Logger::Log("Invalid Packet Error");
                pk->CLEAR_PACKET();
                buffer.Clear();
                break;
            }
            
                // 사용하지 못한 패킷은 다시 풀에 반납 후 루프 탈출 (무한 루프 방지)
                //session->processPool.Push(std::move(pk));
                //break; 
        }

        else
        {
            // 4. 성공 시 패킷 크기만큼 읽기 포인터(read_pos) 전진
            //std::cout << "Success!" << std::endl;
            int PK_LENGTH = pk->GetSerializedSize();
            buffer.OnRead(PK_LENGTH);
        }

        NetworkTask task = {ElementStage::Send, nullptr, pk};
        responsePool->Push(shardID, std::move(task));
    }
}

void TCP_IPC::Sender::Work()
{
    std::vector<NetworkTask> tasks;
    int retval;
    while (isRunning)
    {
        if (requestPool->PopChunk(shardID, tasks, 32)) continue;
        for (auto& task : tasks)
        {
            if (task.pk == nullptr) continue;
            task.pk->Serialize(buffer.GetVector());
        }
        retval = send(sock, buffer.GetBufferToSend(), buffer.Size(), 0);
        if (static_cast<size_t>(retval) == buffer.Size())
        {
            //buffer.IsSending().store(false, std::memory_order_release);
            buffer.Clear();
        }
        else
        {
            buffer.PushFrontRange(retval);
        }
    }
}

bool TCP_IPC::MakeWorkers()
{
    std::unique_ptr<Recver> recver = nullptr;
    std::unique_ptr<Sender> sender = nullptr;
    for (uint32_t i = 0; i < threadCount; i++)
    {
        recver = std::make_unique<Recver>(i, sock, responsePool, pkPool);    
        if (recver == nullptr || !recver->Initialize()) return false;
        recvPool.AddElement(std::move(recver));

        sender = std::make_unique<Sender>(i, sock, requestPool, pkPool);
        if (sender == nullptr || !sender->Initialize()) return false;
        sendPool.AddElement(std::move(sender));    
    }
    return true;
}