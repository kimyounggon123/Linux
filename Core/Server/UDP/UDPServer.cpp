#include "UDPServer.hpp"



void UDPServer::SessionReader::Work()
{
    //std::cout << "Worker Thread [" << std::this_thread::get_id() << "] Start!" << std::endl;
    while (isRunning)
    {
        if (isRecvGateClose == true)
        {
            ThreadUtil::SleepMs(ThreadUtil::Sec);
            continue;
        }

        // 3. 이벤트 발생 대기 (무한 대기)
        // event_count = 데이터를 보낸 사람 수
        int event_count = epoll_wait(epoll_data->epfd, epoll_data->events, MAX_EVENTS, ThreadUtil::Sec);
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


bool UDPServer::SessionReader::ReadLogic()
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

UDPSession* UDPServer::SessionReader::MakeSession()
{
    std::unique_ptr<UDPSession> session = std::make_unique<UDPSession>(ProtocolType::UDP, ConnectState::CONNECT, clientAddr);
    if (session == nullptr) return nullptr;
    UDPSession* retval = session.get();
    udpManager.AddUDPSession(std::move(session));
    if (retval != nullptr) std::cout << "Make Client" << std::endl;
    return retval;
}
void UDPServer::SessionReader::DeleteSession(UDPSession* session)
{
    if (session == nullptr) return;
    if (udpManager.PendDelete(session)) std::cout << "Delete at Searcher" << std::endl;
    else std::cout << "Delete Fail" << std::endl;
    core.sessionManager->PendDelete(session);
}
Packet* UDPServer::SessionReader::MakePacketFromBuffer()
{
    if (buffer.IsEmpty()) 
    {
        return nullptr;
    }
    Packet* pk  = nullptr;
    pk = core.pkPool->Acquire();
    if (pk == nullptr) 
    {
        return nullptr;
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
            return nullptr;
        }

        else
        {
            std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
            // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
            // Logger::Log("Invalid Packet Error");
            pk->CLEAR_PACKET();
            return nullptr;
        }
    }

    else
    {
        // 4. 성공 시 패킷 크기만큼 읽기 포인터(read_pos) 전진
        //std::cout << "Success!" << std::endl;
        int PK_LENGTH = pk->GetSerializedSize();
        buffer.OnRead(PK_LENGTH);
    }
    return pk;
}
bool UDPServer::SessionReader::DeserializeBuffer(UDPSession* session)
{
    if (session == nullptr) 
    {
        std::cout << "null session" << std::endl;
        return false;
    }

    Packet* pk = nullptr;
    while (!buffer.IsEmpty())
    {
        pk = MakePacketFromBuffer();
        if (pk == nullptr) return false;
        // 5. Process pool에게 넘김

        NetworkTask* element = core.taskPool->Acquire();
        if (element == nullptr) continue;

        element->StartTask(ElementStage::Send, session, pk);
        core.processPipePool->Push(session->GetID(), element); 
        session->UpdateHeartbeat();
    }

    return true;
}

int UDPServer::SessionWriter::maxSendCount = 60;
socklen_t UDPServer::SessionWriter::clientAddrLen = sizeof(sockaddr_in); 

void UDPServer::SessionWriter::Work() 
{
    while (isRunning)
    {
        SendNetworkTask();   
        Broadcast();
        ThreadUtil::SleepMs(100);
    }
}


void UDPServer::SessionWriter::SendNetworkTask()
{
    if (!core.sendPipePool->PopChunk(shardID, elementList)) 
    {
        return;
    }

    for (auto it = elementList.begin(); it != elementList.end(); it++)
    {
        NetworkTask* task = *it;
        UDPSession* session = dynamic_cast<UDPSession*>(task->session);
        Packet* pk = task->pk;
        
        if (session == nullptr || pk == nullptr) 
        {
            it = elementList.erase(it); // 안전하게 지우고 다음 반복자 획득
            continue;
        }

        if (!task->pk->Serialize(buffer.GetVector())) continue;
        int retval = sendto(sock, buffer.GetBufferToSend(), buffer.Size(), 0, 
            (struct sockaddr*)&task->session->GetAddr(), clientAddrLen);
        if (retval < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 재시도 대상
            }
            else if (errno == EINTR)
            {
                // 다시 시도 가능
            }
        }
        task->RecordSendTime();

        //task.ShowTimeStamp(false);   
        core.pkPool->Release(pk);
        core.taskPool->Release(task);
        buffer.Clear();
    }
    elementList.clear(); // 기존 pkList는 반드시 비우기.
}

void UDPServer::SessionWriter::Broadcast()
{
    if (!core.broadcastPipePool->PopChunk(shardID, broadcastList)) 
    {
        return;
    }

    Packet* pk = nullptr;
    BasicSession* base = nullptr;
    UDPSession* udpSession = nullptr;
    for (auto* broadcastTask : broadcastList)
    {
        if (broadcastTask == nullptr) continue;

        pk = broadcastTask->pk;
        if (pk == nullptr) continue;
        if (pk->Serialize(broadcastBuffer.GetVector()) != ERROR_CODE::SUCCESS) continue;

        // 리스트에 있는 세션들에게 전송
        for (uint32_t sessionID : broadcastTask->sessionIDList)
        {
            base = udpManager.FindSession(sessionID);
            if (base == nullptr || base->GetProtocolType() != ProtocolType::UDP) continue;
            auto* session = static_cast<UDPSession*>(base);

            session->RefThis();
            int retval = sendto(sock, broadcastBuffer.GetBufferToSend(), broadcastBuffer.Size(), 0, (struct sockaddr*)&session->GetAddr(), clientAddrLen);
            if (retval < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // 재시도 대상
                }
                else if (errno == EINTR)
                {
                    // 다시 시도 가능
                }
            }
            session->ReleaseThisRef();
        }

        // task 정리.
        core.pkPool->Release(pk);
        core.broadPool->Release(broadcastTask);
        broadcastBuffer.Clear();
    }
    broadcastList.clear();
}

bool UDPServer::MakeSessionWorkers() 
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
                (i, sock, epoll_use_this, services, udpManager, isRecvGateClose);
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
