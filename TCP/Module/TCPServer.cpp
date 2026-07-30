#include "TCPServer.hpp"


void TCPServer::SessionReader::Work()
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
        if (event_count == -1) break;
        //std::cout << "event count: " << event_count << std::endl;

        for (int i = 0; i < event_count; i++)
        {
            int current_data_fd = epoll_data->events[i].data.fd;
            
            // [Case 1] 서버 소켓에 이벤트 발생 = 새로운 클라이언트 접속
            if (current_data_fd == epoll_data->sock)
            {   
                HelloNewSession();
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
    //std::cout << "Close this server thread."<< std::endl;
}


bool TCPServer::SessionReader::ReadLogic(TCPSession* session)
{
    RecvBuffer& buffer = session->GetRecvBuffer();
    if (buffer.GetVoidSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
    int str_len = read(session->GetSocket(), buffer.GetBufferToRead(), buffer.GetVoidSpace());
                    
    if (str_len == -1)
    {
        //std::cout << "errno: " << errno <<  std::endl;
        // 넌블로킹 소켓일 때 "지금 당장 읽을 데이터 없음" 에러는 무시
        if (errno == EAGAIN || errno == EWOULDBLOCK)  return true;
                            
        // 그 외의 진짜 에러는 끊긴 것으로 처리
        str_len = 0; 
    }

    // 연결 종료 요청
    if (str_len == 0) 
    { 
        ByeSession(session);
        return false;
    }
    else 
    {
        buffer.OnWrite(str_len);
        ProcessClientBuffer(session);
        //write(current_fd, session->send_buffer.buffer, (BUFFERSIZE + 1));
    }

    return true;
}

void TCPServer::SessionReader::HelloNewSession()
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
        std::cout << "clnt_sock error! " << std::endl;
        return;
    } 

    // int socket_fd, const ProtocolType& type, const ConnectState& state, const struct sockaddr_in& addr
    auto new_session = std::make_unique<TCPSession>(clnt_sock, ProtocolType::TCP, ConnectState::CONNECT, clnt_addr);
    //if (!new_session->Start()) return;
    TCPSession* session_ptr = new_session.get();

    context.sessionManager.AddSessionInBasicMap(std::move(new_session));    
    session_ptr->RefThis();

    // 3. epoll 등록 구조체 설정
    struct epoll_event ev; // 루프 공용 'event' 대신 지역변수 'ev'를 쓰는 게 안전합니다.
    ev.events = session_ptr->GetEPOLLEvents(); // 내부에서 초기화된 EPOLLIN (즉, 32)    
    ev.data.ptr = session_ptr;
    if (epoll_ctl(epoll_data->epfd, EPOLL_CTL_ADD, clnt_sock, &ev) == -1)
    {
        perror("epoll_ctl");

        session_ptr->ReleaseThisRef();
        context.sessionManager.PendDelete(session_ptr);
        return;
    }
    //std::cout << "New client connected(socket_id): " << clnt_sock <<  std::endl;
}

void TCPServer::SessionReader::ByeSession(TCPSession* session)
{
    epoll_ctl(epoll_data->epfd, EPOLL_CTL_DEL, session->GetSocket(), NULL);
    //std::cout << "Client disconnected: " << session->GetSocket() << std::endl;
    session->ReleaseThisRef(); // 해당 서버가 이제 참조를 그만 둠.
    context.sessionManager.PendDelete(session);
}

void TCPServer::SessionReader::ProcessClientBuffer(TCPSession* session)
{
    // std::cout << "receved data: " << recvLength <<  std::endl; 
    // std::cout << "read buffer data: " << "(read_pos): " << session->recv_buffer.read_pos 
    //     << " (write_pos): " << session->recv_buffer.write_pos << std::endl; 
    // for (int i = session->recv_buffer.read_pos; i < session->recv_buffer.read_pos + recvLength; i++)
    // {
    //     printf("%02X ", session->recv_buffer.buffer[i]);
    // }
    // printf("\n");
    
    RecvBuffer& buffer = session->GetRecvBuffer();
    Packet* pk = nullptr;
    size_t offset = 0;
    while (!buffer.IsEmpty())
    {
        offset = 0;
        // 1. 오브젝트 풀에서 패킷 객체 할당 실패 시 루프 탈출
        pk = context.packetPool.Acquire();
        if (pk == nullptr) 
        {
            //std::cout << "Empty Containor." << std::endl;
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
                context.packetPool.Release(pk);
                break;
            }

            else
            {
                std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
                // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
                // Logger::Log("Invalid Packet Error");
                pk->CLEAR_PACKET();
                buffer.Clear();
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

        // 5. Process pool에게 넘김
        NetworkTask element = {ElementStage::GeneralProcess, session, pk};
        context.router.EnqueueElement(PipeType::ProcessInput, session->GetID(), std::move(element));   
    }  
}

int TCPServer::SessionWriter::maxSendCount = 60;

void TCPServer::SessionWriter::Work() 
{
    std::vector<NetworkTask> elementList;
    while (isRunning)
    {
        if (!context.router.DequeueElementAsChunk(PipeType::SendThis, shardID, elementList)) 
        {
            //std::cout << "cannot found session in writer" << std::endl;
            continue;
        }
        // 패킷 꺼내오기
        
        for (auto it = elementList.begin(); it != elementList.end();)
        {
            NetworkTask& element = *it;
            TCPSession* session = dynamic_cast<TCPSession*>(element.session);
            Packet* pk = element.pk;
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
                pk->Serialize(buffer.GetVector());
                Write(session); // 이 안에서 실제 send() 수행
                element.RecordSendTime();

                element.ShowTimeStamp(false);
                context.packetPool.Release(pk); it++;
            } 
            else 
            {
                // [실패] 이 세션은 지금 전송 중임. 
                // 패킷을 그대로 다시 라우터로 돌려보내서 다음 루프 때 처리하게 만듦!
                // 보통은 이렇게 안 하고 세션 자체에 큐를 만듦.
                context.router.EnqueueElement(PipeType::SendThis, shardID, std::move(element));
                it = elementList.erase(it);
            }
        }
        elementList.clear(); // 기존 pkList는 반드시 비우기.
    }
}

int TCPServer::SessionWriter::Write(TCPSession* session)
{
    SendBuffer& buffer = session->GetSendBuffer();
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
    return retval;
}

TCPServer::TCPServer(uint16_t port): BaseServer(false, port)
{}

bool TCPServer::Initialize()
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
            epoll_pointer->ChangeStyle(false);
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



// LinuxServer::LinuxServer(): isRunning(true), tcp(nullptr)
// {}
// LinuxServer::~LinuxServer()
// {
//     Destroy();
// }

// void LinuxServer::Destroy()
// {
//     SAFE_FREE(tcp);
//     //SAFE_FREE(redis_manager);
// }


// bool LinuxServer::Initialize(uint16_t port)
// {   
//     tcp = new TCPserver(port);
//     if (tcp == nullptr || !tcp->Initialize()) return false;

//     //redis_manager = new RedisManager("tcp://127.0.0.1:6000");
//     //if (redis_manager == nullptr || redis_manager->Initialize()) return false;

//     std::cout<< "Initialize Complete." << std::endl;
//     return true;
// }

// void LinuxServer::Run()
// {
//     std::cout<< "=============Start Server!=============" << std::endl;
//     //std::cout<< "You should press Ctrl + C to quit this runfile." << std::endl;
//     std::string command;
//     while (isRunning)
//     {
//         /*
//         std::cout << "Server# ";
//         std::cin >> command;

//         if (command == "exit" || command == "quit")
//         {
//             std::cout << "서버 종료 명령을 수신했습니다. 안전 종료를 시작합니다..." << std::endl;
//             isRunning = false;
//         }
//         else if (command == "status")
//         {
//             std::cout << "[STATUS] 현재 가동 중인 recver 스레드 수: " << recverPool->Size() << std::endl;
//             std::cout << "[STATUS] 현재 가동 중인 sender 스레드 수: " << senderPool->Size() << std::endl;
//             std::cout << "[STATUS] 현재 가동 중인 process worker 스레드 수: " << processPool->Size() << std::endl;
//         }
//         else
//         {
//             std::cout << "알 수 없는 명령입니다. (status, exit 중 입력)" << std::endl;
//         }
//         */
//         sleep(1);
//     }

//     std::cout << "Close this server."<< std::endl;
// }