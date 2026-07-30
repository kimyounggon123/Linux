#include "UDPServer.hpp"



void UDPserver::SessionReader::Work()
{
    //std::cout << "Worker Thread [" << std::this_thread::get_id() << "] Start!" << std::endl;
    while (isRunning)
    {
        // 3. 이벤트 발생 대기 (무한 대기)
        // event_count = 데이터를 보낸 사람 수
        int event_count = epoll_wait(epoll_data->epfd, epoll_data->events, MAX_EVENTS, 1000);
        if (event_count == 0) 
        {
            //std::cout << "cannot found session in reader" << std::endl;
            continue;
        }
        //if (event_count == -1) break;
        //std::cout << "event count: " << event_count << std::endl;

        for (int i = 0; i < event_count; i++)
        {
            //int current_data_fd = epoll_data->events[i].data.fd;
                // 1. ET Style
            if (epoll_data->ET_style == true)
            {
                while (true)
                {
                    if (!ReadLogic()) break;
                }
            }
            else
            {
                ReadLogic();
            }
        }
    }

    //std::cout << "Close this server thread."<< std::endl;
}
    
bool UDPserver::SessionReader::ReadLogic()
{
    if (buffer.GetVoidSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
    socklen_t clientAddrLen = sizeof(clientAddr); 
    int str_len =  recvfrom(sock, buffer.GetWritePtr(), buffer.GetVoidSpace(), 0,
            (struct sockaddr*)&clientAddr, 
            &clientAddrLen);
    if (str_len == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        else  return false;
    }
    else ProcessClientBuffer(str_len, clientAddr);   

    return true;
}   


void UDPserver::SessionReader::ProcessClientBuffer(int recvLength, const struct sockaddr_in& addr)
{

    Packet* pk = context.packetPool.Acquire();
    if (pk == nullptr) return;

    pk->ClearBuffer();
    ERROR_CODE code = pk->Deserialize(buffer.GetReadPtr(), recvLength);
        
    if (code != ERROR_CODE::SUCCESS) // 또는 !code (SUCCESS가 0인 경우)
    {
        std::cout << "Deserialize fail" << std::endl;
        if (code == ERROR_CODE::NEED_EXTRA_DATA)
        {
            std::cout << "NEED_EXTRA_DATA" << std::endl;
            // 데이터가 더 필요하므로, 남은 데이터를 버퍼 앞쪽으로 당기고 다음 recv를 기다림
            buffer.MoveDataFront();
            context.packetPool.Release(pk);
        }

        else
        {
            std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
            // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
            // Logger::Log("Invalid Packet Error");
            pk->CLEAR_PACKET();
            //buffer.Clear();
        }
    }

    else
    {
        // 4. 성공 시 패킷 크기만큼 읽기 포인터(read_pos) 전진
        //std::cout << "Success!" << std::endl;
        int PK_LENGTH = pk->GetSerializedSize();
        buffer.OnRead(PK_LENGTH);
    }

    // 5. Process pool에게 넘김
    NetElement element = {ElementStage::GeneralProcess, addr, pk};
    context.router.EnqueueElement(PipeType::ProcessInput,   std::move(element));  
}

int UDPserver::SessionWriter::maxSendCount = 60;

void UDPserver::SessionWriter::Work() 
{
    std::vector<NetElement> elementList;
    while (isRunning)
    {
        if (!context.router.DequeueElementAsChunk(PipeType::SendThis, shardID, elementList)) 
        {
            //std::cout << "cannot found session in writer" << std::endl;
            continue;
        }
        // 패킷 꺼내오기
        
        for (auto it = elementList.begin(); it != elementList.end(); it++)
        {
            NetElement& element = *it;
            UDPSession* session = dynamic_cast<UDPSession*>(element.session);
            Packet* pk = element.pk;
            if (session == nullptr || pk == nullptr) 
            {
                it = elementList.erase(it); // 안전하게 지우고 다음 반복자 획득
                continue;
            }
            Write(element); // 이 안에서 실제 send() 수행
            element.RecordSendTime();
                
            double time1 = element.GetDurationBetweenRecvAndProcess().count();
            double time2 = element.GetDurationBetweenProcessAndSend().count();
            std::cout << "between recv and process: " << time1 << " (ms)" << std::endl;
            std::cout << "between process and send: " << time2 << " (ms)\n" << std::endl;
            context.packetPool.Release(pk);
        }
        elementList.clear(); // 기존 pkList는 반드시 비우기.
    }
}

int UDPserver::SessionWriter::Write(const NetElement& element)
{
    if (!element.pk->Serialize(buffer.GetVector())) return -1;
    socklen_t clientAddrLen = sizeof(element.addr); 
    int retval = sendto(sock, buffer.GetBufferToSend(), buffer.Size(), 0,
            (struct sockaddr*)&element.addr, clientAddrLen);
    if (retval > 0)
    {
        if (static_cast<size_t>(retval) == buffer.Size())
        {
            buffer.Clear();
        }
        else
        {
            buffer.PushFrontRange(retval);
        }
    }
    return retval;
}

UDPserver::UDPserver(uint16_t port): BaseServer(true, port)
{}

bool UDPserver::Initialize()
{
    if (!BaseServer::Initialize()) return false;
    std::unique_ptr<SessionReader> reader = nullptr;
    std::unique_ptr<SessionWriter> writer = nullptr; 
    try
    {
        uint32_t shardID = 0;
        for (auto& epoll_unique : epoll_pool)
        {
            EPOLL_DATA_REUSEPORT* epoll_pointer = epoll_unique.get();
            epoll_pointer->ChangeStyle(true);
            reader = std::make_unique<SessionReader>(shardID, epoll_pointer, context);
            if (reader == nullptr || reader->Initialize() == false)
            {
                throw false;
            }
            else
            {
                //std::cout << "Reader " << i << " is Ready" << std::endl;    
                //sleep(1);
            }

            writer = std::make_unique<SessionWriter>(shardID, epoll_pointer, context); 
            if (writer == nullptr || writer->Initialize() == false) 
            {
                throw false;
            }
            else 
            {
                //std::cout << "Writer " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            
            //std::cout << "Push " << i << " Start" << std::endl;  
            recverPool.AddElement(std::move(reader));
            senderPool.AddElement(std::move(writer));
            //std::cout << "Push " << i << " is Ready" << std::endl; 
            
            shardID++;
        }
    }
    catch(...)
    {
        return false;
    }
    
    return true;
}