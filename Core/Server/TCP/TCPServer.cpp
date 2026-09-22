#include "TCPServer.hpp"

void TCPServer::SessionReader::Work()
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
            //std::cout << "cannot found session in reader" << std::endl;
            continue;
        }
        if (event_count == -1) break;
        //std::cout << "event count: " << event_count << std::endl;

        for (int i = 0; i < event_count; i++)
        {
            int current_data_fd = epoll_data->events[i].data.fd;
            
            // [Case 1] 서버 소켓에 이벤트 발생 = 새로운 클라이언트 접속
            if (current_data_fd == epoll_data->sock)
            {   
                MakeSession();
                continue;
            }

            // [Case 2] 클라이언트 소켓에 이벤트 발생 = 데이터 수신 또는 연결 종료 
            //  read - deserialize - process - serialize - write
            // deserialize 부분에서 여러 패킷을 받을 수 있으므로 이를 고려하면서 처리해야 함.
            else 
            {
                TCPSession* session = static_cast<TCPSession*>(epoll_data->events[i].data.ptr);
                if (session == nullptr) 
                {
                    std::cout << "Session is nullptr!" <<  std::endl;
                    continue;
                }

                if (session->IsPendDelete()) continue;

                // 1. ET Style
                if (epoll_data->ET_style == true)
                {
                    while (true)
                    {
                        if (!ReadLogic(session)) break;
                    }
                }
                else
                {
                    // 2. LT Style
                    ReadLogic(session);
                }       
            }
        }
    }
    //std::cout << "Close this server thread. "<< shardID << std::endl;
}

bool TCPServer::SessionReader::ReadLogic(TCPSession* session)
{
    //LogTool::Log("Reader Work", "start");

    bool retval = true;

    RecvBuffer& buffer = session->GetRecvBuffer();
    if (buffer.GetVoidSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
    int str_len = read(session->GetSocket(), buffer.GetBufferToRead(), buffer.GetVoidSpace());
    if (str_len == -1)
    {
        //std::cout << "errno: " << errno <<  std::endl;
        // 넌블로킹 소켓일 때 "지금 당장 읽을 데이터 없음" 에러는 무시
        if (errno == EAGAIN || errno == EWOULDBLOCK)  return false;
                            
        // 그 외의 진짜 에러는 끊긴 것으로 처리
        str_len = 0; 
    }

    // 연결 종료 요청
    if (str_len == 0) 
    { 
        DeleteSession(session);
        retval = false;
    }
    else 
    {
        buffer.OnWrite(str_len);
        retval = DeserializeBuffer(session);
        //write(current_fd, session->send_buffer.buffer, (BUFFERSIZE + 1));
    }
    return retval;
}

void TCPServer::SessionReader::MakeSession()
{
    struct sockaddr_in clnt_addr;
    socklen_t addr_sz = sizeof(clnt_addr);

    int clnt_sock = accept4(
        epoll_data->sock,
        (sockaddr*)&clnt_addr,
        &addr_sz,
        SOCK_NONBLOCK
    );
    if (clnt_sock == -1) // 예외
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) // 더 받을 연결 없음
        {
            //std::cout << "EAGAIN || EWOULDBLOCK" << std::endl;
            return; 
        }
        if (errno == EINTR) 
        {
            //std::cout << "EINTR" << std::endl;
            return; // 또는 accept 다시 시도
        } 
        std::cerr << "accept4 error: "
                << strerror(errno)
                << " (" << errno << ")\n";
        return;
    } 

    // int socket_fd, const ProtocolType& type, const ConnectState& state, const struct sockaddr_in& addr
    auto new_session = std::make_unique<TCPSession>(clnt_sock, ProtocolType::TCP, ConnectState::CONNECT, clnt_addr, useHeartbeats);
    TCPSession* session_ptr = new_session.get();

    services.sessionManager->AddSessionInBasicMap(std::move(new_session));    
    session_ptr->RefThis();

    // 3. epoll 등록 구조체 설정
    struct epoll_event ev; // 루프 공용 'event' 대신 지역변수 'ev'를 쓰는 게 안전합니다.
    ev.events = session_ptr->GetEPOLLEvents(); // 내부에서 초기화된 EPOLLIN (즉, 32)    
    ev.data.ptr = session_ptr;
    if (epoll_ctl(epoll_data->epfd, EPOLL_CTL_ADD, clnt_sock, &ev) == -1)
    {
        perror("epoll_ctl");
        session_ptr->ReleaseThisRef();
        services.sessionManager->PendDelete(session_ptr);
        return;
    }
    std::cout << "New client connected(socket_id): " << clnt_sock <<  std::endl;
}

void TCPServer::SessionReader::DeleteSession(TCPSession* session)
{
    epoll_ctl(epoll_data->epfd, EPOLL_CTL_DEL, session->GetSocket(), NULL);
    //std::cout << "Client disconnected: " << session->GetSocket() << std::endl;
    session->ReleaseThisRef(); // 해당 서버가 이제 참조를 그만 둠.
    services.sessionManager->PendDelete(session);
}

Packet* TCPServer::SessionReader::MakePacketFromBuffer(RecvBuffer& buffer)
{
    Packet* pk = nullptr;
    pk = services.pkPool->Acquire();
    if (pk == nullptr) 
    {
        LogTool::Log("Reader Work", "packet is null");
        return nullptr;
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
            services.pkPool->Release(pk);
            return nullptr;
        }

        else
        {
            std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
            // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
            // Logger::Log("Invalid Packet Error");
            pk->CLEAR_PACKET();
            buffer.Clear();
            return nullptr;
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
    return pk;
}

bool TCPServer::SessionReader::DeserializeBuffer(TCPSession* session)
{    
    if (session == nullptr) return false;
    
    RecvBuffer& buffer = session->GetRecvBuffer();
    Packet* pk = nullptr;
    while (!buffer.IsEmpty())
    {
        pk = MakePacketFromBuffer(buffer);
        if (pk == nullptr) return false;

        NetworkTask* element = services.taskPool->Acquire();
        if (element == nullptr) continue;

        element->StartTask(ElementStage::Send, session, pk);
        services.processPipePool->Push(session->GetID(), element); 
        session->UpdateHeartbeat();
    }  
    return true;
}

int TCPServer::SessionWriter::maxSendCount = 60;

void TCPServer::SessionWriter::Work() 
{
    while (isRunning)
    {
        SendNetworkTask();
        Broadcast();
        ThreadUtil::SleepMs(10);
    }
}

void TCPServer::SessionWriter::SendNetworkTask()
{
    if (!services.sendPipePool->PopChunk(shardID, elementList)) 
    {
        return;
    }
    // 패킷 꺼내오기
    for (auto it = elementList.begin(); it != elementList.end();)
    {
        NetworkTask* element = *it;
        TCPSession* session = dynamic_cast<TCPSession*>(element->session);
        Packet* pk = element->pk;
        SendBuffer& buffer = session->GetSendBuffer();
        if (session == nullptr || pk == nullptr) 
        {
            it = elementList.erase(it); // 안전하게 지우고 다음 반복자 획득
            continue;
        }

        bool expected = false;
        if (buffer.IsSending().compare_exchange_strong(expected, true, std::memory_order_acquire)) 
        {
            // [성공] 내가 전송 권한을 얻었으므로 버퍼에 쓰고 발송!
            if (pk->Serialize(buffer.GetVector()) != ERROR_CODE::SUCCESS) continue;
            //std::cout << "send return: " << buffer.Size() << std::endl;

            int retval = write(session->GetSocket(), buffer.GetBufferToSend(), buffer.Size());
            if (retval > 0)
            {
                if (static_cast<size_t>(retval) == buffer.Size())
                {
                    buffer.IsSending().store(false, std::memory_order_release);
                    buffer.Clear();
                }
                else
                {
                    buffer.PushFrontRange(retval);
                }
            }
            //std::cout << "send return: " << retval << std::endl;
            element->RecordSendTime();

            //element.ShowTimeStamp(false);
            services.pkPool->Release(pk); 
            services.taskPool->Release(element);
            it++;

        } 
        else 
        {
            // [실패] 이 세션은 지금 전송 중임. 
            // 패킷을 그대로 다시 라우터로 돌려보내서 다음 루프 때 처리하게 만듦!
            services.sendPipePool->Push(shardID, element);
            it = elementList.erase(it);
        }
    }
    elementList.clear(); // 기존 pkList는 반드시 비우기.
}
void TCPServer::SessionWriter::Broadcast()
{
    if (!services.broadcastPipePool->PopChunk(shardID, broadcastList)) 
    {
        return;
    }    

    Packet* pk = nullptr;
    BasicSession* base = nullptr;
    TCPSession* tcpSession = nullptr;
    for (auto* broadcastTask : broadcastList)
    {
        if (broadcastTask == nullptr) continue;

        // serialize
        pk = broadcastTask->pk;
        if (pk == nullptr) continue;
        if (pk->Serialize(broadcastBuffer.GetVector()) != ERROR_CODE::SUCCESS) continue;

        // 리스트에 있는 세션들에게 전송
        for (uint32_t sessionID : broadcastTask->sessionIDList)
        {
            base = services.sessionManager->FindSession(sessionID);
            if (base == nullptr || base->GetProtocolType() != ProtocolType::TCP) continue;
            auto* session = static_cast<TCPSession*>(base);

            session->RefThis();
            int retval = write(session->GetSocket(), broadcastBuffer.GetBufferToSend(), broadcastBuffer.Size());
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
        services.pkPool->Release(pk);
        services.broadPool->Release(broadcastTask);
        broadcastBuffer.Clear();
    }

    broadcastList.clear();
}


bool TCPServer::MakeSessionWorkers() 
{
    managers.AddManager(&sessionManager);

    std::unique_ptr<SessionReader> reader = nullptr;
    std::unique_ptr<SessionWriter> writer = nullptr;
    try
    {
        uint32_t shardID = 0;
        for (auto& epoll_unique : epoll_pool)
        {
            EPOLL_DATA_REUSEPORT* epoll_pointer = epoll_unique.get();
            epoll_pointer->ChangeStyle(true);
            reader = std::make_unique<SessionReader>(shardID, epoll_pointer, services, useHeartbeats, isRecvGateClose);
            if (reader == nullptr || reader->Initialize() == false)
            {
                throw false;
            }
            else
            {
                //std::cout << "Reader " << i << " is Ready" << std::endl;    
                //sleep(1);
            }

            writer = std::make_unique<SessionWriter>(shardID, epoll_pointer, services); 
            if (writer == nullptr || writer->Initialize() == false) 
            {
                throw false;
            }
            else 
            {
                //std::cout << "Writer " << i << " is Ready" << std::endl;    
                //sleep(1);
            }

            //aoiWorker = std::make_unique<AOIEventWorker>(gameElement, shardID);
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
