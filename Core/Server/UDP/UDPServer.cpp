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
            continue;
        }
        //if (event_count == -1) break;
        //std::cout << "event count: " << event_count << std::endl;
        for (int i = 0; i < event_count; i++)
        {
            int current_data_fd = epoll_data->events[i].data.fd;
            if (current_data_fd == epoll_data->sock)
            {   
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
    }

    //std::cout << "Close this server thread."<< std::endl;
}


bool UDPserver::SessionReader::ReadLogic()
{
    if (buffer.GetVoidSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
    socklen_t clientAddrLen = sizeof(clientAddr); 
    
    int str_len =  recvfrom(sock, buffer.GetBufferToRead(), buffer.GetVoidSpace(), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (str_len == -1)
    {
        // std::cerr
        // << "recvfrom failed. "
        // << "sock=" << sock
        // << ", errno=" << errno
        // << ", error=" << strerror(errno)
        // << std::endl;

        if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
        str_len = 0;
    }

    buffer.OnWrite(str_len);
    Endpoint point = {clientAddr};
    UDPSession* session = udpManager.FindUDP(point);
    if (session == nullptr) 
    {
        session = MakeSession();
    }
    
    session->SetEndPoint(clientAddr); // 임시 테스트용
    return DeserializeBuffer(session);
}   

UDPSession* UDPserver::SessionReader::MakeSession()
{
    std::unique_ptr<UDPSession> session = std::make_unique<UDPSession>(ProtocolType::UDP, ConnectState::CONNECT, clientAddr);
    if (session == nullptr) return nullptr;
    UDPSession* retval = session.get();
    udpManager.AddUDPSession(std::move(session));
    if (retval != nullptr) std::cout << "Make Client" << std::endl;
    return retval;
}
void UDPserver::SessionReader::DeleteSession(UDPSession* session)
{
    if (session == nullptr) return;
    if (udpManager.PendDelete(session)) std::cout << "Delete at Searcher" << std::endl;
    else std::cout << "Delete Fail" << std::endl;
    core.sessionManager->PendDelete(session);
}

bool UDPserver::SessionReader::DeserializeBuffer(UDPSession* session)
{
    if (buffer.IsEmpty()) 
    {
        std::cout << "empty buffer" << std::endl;
        return false;
    }
    if (session == nullptr) 
    {
        std::cout << "null session" << std::endl;
        return false;
    }

    Packet* pk = core.pkPool->Acquire();
    if (pk == nullptr) 
    {
        return false;
    }

    pk->ClearBuffer();
    int rcvLen = buffer.GetCurrDataSize();
    ERROR_CODE code = pk->Deserialize(buffer.GetReadPtr(), rcvLen);
    
    if (code != ERROR_CODE::SUCCESS) // 또는 !code (SUCCESS가 0인 경우)
    {
        std::cout << "Deserialize fail" << std::endl;
        if (code == ERROR_CODE::NEED_EXTRA_DATA)
        {
            std::cout << "NEED_EXTRA_DATA" << std::endl;
            buffer.MoveDataFront();
            core.pkPool->Release(pk);
            return false;
        }

        else
        {
            std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
            // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
            // Logger::Log("Invalid Packet Error");
            pk->CLEAR_PACKET();
            return false;
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
    NetworkTask task = {ElementStage::Send, session, pk};
    core.processPipePool->Push(shardID, std::move(task)); 
    
    return true;
}

int UDPserver::SessionWriter::maxSendCount = 60;

void UDPserver::SessionWriter::Work() 
{
    std::vector<NetworkTask> elementList;
    while (isRunning)
    {
        if (!core.sendPipePool->PopChunk(shardID, elementList)) 
        {
            //std::cout << "cannot found session in writer" << std::endl;
            continue;
        }
        // 패킷 꺼내오기
        
        for (auto it = elementList.begin(); it != elementList.end(); it++)
        {
            NetworkTask& task = *it;
            UDPSession* session = dynamic_cast<UDPSession*>(task.session);
            Packet* pk = task.pk;
            if (session == nullptr || pk == nullptr) 
            {
                it = elementList.erase(it); // 안전하게 지우고 다음 반복자 획득
                continue;
            }
            Write(task); // 이 안에서 실제 send() 수행
            task.RecordSendTime();
            //task.ShowTimeStamp(false);   
            core.pkPool->Release(pk);
        }
        elementList.clear(); // 기존 pkList는 반드시 비우기.
    }
}

int UDPserver::SessionWriter::Write(const NetworkTask& task)
{
    if (!task.pk->Serialize(buffer.GetVector())) return -1;
    socklen_t clientAddrLen = sizeof(sockaddr_in); 
    int retval = sendto(sock, buffer.GetBufferToSend(), buffer.Size(), 0,
            (struct sockaddr*)&task.session->GetAddr(), clientAddrLen);
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
    // std::cout << "Retval: " << retval << std::endl;
    // else 
    // {
    //     std::cerr << "errno: " << errno << std::endl;
    // }

    return retval;
}


bool UDPserver::MakeSessionWorkers() 
{
    services.sessionManager = &udpManager;
    managers.Clear();
    managers.AddManager(&udpManager);

    epoll_use_this = epoll_pool.data()->get();
    if (epoll_use_this == nullptr) return false;
    epoll_use_this->ChangeStyle(true);
    
    std::unique_ptr<SessionReader> reader = nullptr;
    std::unique_ptr<SessionWriter> writer = nullptr; 
    try
    {
        for (uint32_t i = 0; i < threadPoolCount; i++)
        {
            reader = std::make_unique<SessionReader>
                (i, sock, epoll_use_this, services, udpManager);
            if (reader == nullptr || reader->Initialize() == false)
            {
                throw false;
            }
            else
            {
                //std::cout << "Reader " << i << " is Ready" << std::endl;    
                //sleep(1);
            }

            writer = std::make_unique<SessionWriter>
                (i, sock, epoll_use_this, services, udpManager); 
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
            
        }

    }
    catch(...)
    {
        return false;
    }
    return true;
}
