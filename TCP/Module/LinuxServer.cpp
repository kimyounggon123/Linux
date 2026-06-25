#include "LinuxServer.hpp"

bool EPOLL_DATA_REUSEPORT::Initialize(int serverSocket)
{
    if (isAlive) return true;

    // epoll 인스턴스 생성
    epfd = epoll_create(1); 

    sock = serverSocket;

    // 서버 소켓(Listen 소켓)을 epoll에 등록
    event.events = EPOLLIN; // 데이터 수신(접속 요청)을 관찰
    if (ET_style) event.events = event.events | EPOLLET;
    
    event.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event);

    isAlive = true;
    return true;
}

void EPOLL_DATA_REUSEPORT::Destroy()        
{
    if (epfd != -1) close(epfd);
    isAlive = false;
}

IServer::IServer(uint16_t port): port(port), sock(-1), addr{},
    recverPool(nullptr), senderPool(nullptr), processPool(nullptr), process(nullptr),
    dbProcessPool(nullptr), dbProcess(nullptr)
{
    
}
IServer::~IServer()
{
    Destroy();
}

void IServer::Destroy()
{
    if (recverPool) 
    {
        recverPool->StopAll();
        delete recverPool;
        recverPool = nullptr;
    }
    if (senderPool)
    {
        senderPool->StopAll();
        delete senderPool;
        senderPool = nullptr;
    }
    if (processPool)
    {
        processPool->StopAll();
        delete processPool;
        processPool = nullptr;
    } 
    if (dbProcessPool)
    {
        dbProcessPool->StopAll();
        delete dbProcessPool;
        dbProcessPool = nullptr;
    }

    SAFE_FREE(process);
    SAFE_FREE(dbProcess);
    epoll_pool.clear();

    if (sock != -1) close(sock);
}




void TCPserver::Destroy()
{
    IServer::Destroy();
    router.DeleteInstance();
}

void TCPserver::SessionReader::Work()
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
                LinuxSession* session = static_cast<LinuxSession*>(epoll_data->events[i].data.ptr);
                if (session == nullptr) 
                {
                    std::cout << "Session is nullptr!" <<  std::endl;
                    continue;
                }

                if (session->isPendingDelete) continue;

                // 1. ET Style
                session->AddRef();
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


bool TCPserver::SessionReader::ReadLogic(LinuxSession* session)
{
    if (session->recv_buffer.GetFreeSpace() < Packet::MAX_SIZE) session->recv_buffer.MoveDataFront();
    int str_len = read(session->socket_fd, session->recv_buffer.GetWritePtr(), session->recv_buffer.GetFreeSpace());
                    
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
        ByeSession(session);
        return false;
    }
    else 
    {
        session->recv_buffer.OnWrite(str_len);
        ProcessClientBuffer(session, str_len);
        //write(current_fd, session->send_buffer.buffer, (BUFFERSIZE + 1));
    }

    return true;
}

void TCPserver::SessionReader::HelloNewSession()
{
    struct sockaddr_in clnt_addr;
    socklen_t addr_sz = sizeof(clnt_addr);
    int clnt_sock = accept(epoll_data->sock, (struct sockaddr*)&clnt_addr, &addr_sz);
    if (clnt_sock == -1) // 예외
    {
        std::cout << "clnt_sock error! " << std::endl;
        return;
    } 
                
    auto new_session = std::make_unique<LinuxSession>(clnt_sock);
    LinuxSession* session_ptr = new_session.get();

    // 3. epoll 등록 구조체 설정
    struct epoll_event ev; // 루프 공용 'event' 대신 지역변수 'ev'를 쓰는 게 안전합니다.
    ev.events = session_ptr->epoll_events; // 내부에서 초기화된 EPOLLIN (즉, 32)    
    ev.data.ptr = session_ptr;
    epoll_ctl(epoll_data->epfd, EPOLL_CTL_ADD, clnt_sock, &ev);
    
    new_session->Start();
    sessionManager.AddSessionInBasicMap(std::move(new_session));

    std::cout << "New client connected(socket_id): " << clnt_sock <<  std::endl;
}

void TCPserver::SessionReader::ByeSession(LinuxSession* session)
{
    epoll_ctl(epoll_data->epfd, EPOLL_CTL_DEL, session->socket_fd, NULL);
    std::cout << "Client disconnected: " << session->socket_fd << std::endl;
    sessionManager.PendDelete(session);
}

void TCPserver::SessionReader::ProcessClientBuffer(LinuxSession* session, int recvLength)
{
    /*
    std::cout << "receved data: " << recvLength <<  std::endl; 
    std::cout << "read buffer data: " << "(read_pos): " << session->recv_buffer.read_pos 
        << " (write_pos): " << session->recv_buffer.write_pos << std::endl; 
    for (int i = session->recv_buffer.read_pos; i < session->recv_buffer.read_pos + recvLength; i++)
    {
        printf("%02X ", session->recv_buffer.buffer[i]);
    }
    printf("\n");
    */
    
    PacketWithOwner* pkWithOwner = nullptr;
    size_t offset = 0;
    while (!session->recv_buffer.IsEmpty())
    {
        offset = 0;
        // 1. 오브젝트 풀에서 패킷 객체 할당 실패 시 루프 탈출
        if (!session->processPool.Pop(pkWithOwner)) 
        {
            std::cout << "Pop fail" << std::endl;
            break; 
        }
        
        // 현재 버퍼에 남아있는 데이터의 총 크기 계산
        // 2. 역직렬화 시도
        Packet& pk = pkWithOwner->pk;
        pk.CLEAR_PACKET();
        ERROR_CODE code = pk.Deserialize(session->recv_buffer.GetReadPtr(), recvLength, offset);
        
        // 3. 역직렬화 실패 처리
        if (code != ERROR_CODE::SUCCESS) // 또는 !code (SUCCESS가 0인 경우)
        {
            std::cout << "Deserialize fail" << std::endl;
            if (code == ERROR_CODE::NEED_EXTRA_DATA)
            {
                std::cout << "NEED_EXTRA_DATA" << std::endl;
                // 데이터가 더 필요하므로, 남은 데이터를 버퍼 앞쪽으로 당기고 다음 recv를 기다림
                session->recv_buffer.MoveDataFront();
                pkWithOwner->ReturnToOwner();
                break;
            }

            else
            {
                std::cout << "ERROR: " << static_cast<uint32_t>(code) <<  std::endl;
                // 그 외의 치명적인 에러 (패킷 변조, 잘못된 헤더 등) -> 세션 종료 등의 처리 필요
                // Logger::Log("Invalid Packet Error");
                pk.CLEAR_PACKET();
                pk.SetType(PacketType::ERROR_TYPE);
                pk.SetResult(PacketResult::Fail);
                session->recv_buffer.Clear();
            }
        
            // 사용하지 못한 패킷은 다시 풀에 반납 후 루프 탈출 (무한 루프 방지)
            //session->processPool.Push(std::move(pk));
            //break; 
        }

        else
        {
            // 4. 성공 시 패킷 크기만큼 읽기 포인터(read_pos) 전진
            //std::cout << "Success!" << std::endl;
            int PK_LENGTH = pk.GetSerializedSize();
            session->recv_buffer.OnRead(PK_LENGTH);
        }

        // 5. Process pool에게 넘김
        session->processContainor.Push(pkWithOwner);
    }
    router.EnqueueSession(PipeID::RecvToProcess, std::move(session), ID);     
}

int TCPserver::SessionWriter::maxSendCount = 60;

void TCPserver::SessionWriter::Work() 
{
    int sendPacketCount;
    LinuxSession* session;
    std::vector<PacketWithOwner*> pkList;
    while (isRunning)
    {
        sendPacketCount = 0;
        session = nullptr;

        if (!router.DequeueSession(PipeID::ProcessToSend, session, ID)) 
        {
            //std::cout << "cannot found session in writer" << std::endl;
            continue;
        }
        // 패킷 꺼내오기
        if (!session->sendContainor.Swap(pkList)) 
        {
            //std::cout << "Is empty!" << std::endl;
            continue;
        }
        
        size_t actualSendCount = std::min(pkList.size(), (size_t)maxSendCount);
        for (size_t i = 0; i < actualSendCount; i++)
        {
            Packet& pk = pkList[i]->pk;
            pk.Serialize(session->send_buffer.GetVector());
            pkList[i]->ReturnToOwner();
            //session->processPool.Push(pkList[i]);
        }

        Write(session);
        if (actualSendCount < pkList.size())
            session->sendContainor.PushFrontRange(std::move(pkList), actualSendCount);

        session->Release();
        
        pkList.clear(); // 기존 pkList는 반드시 비우기.
    }
}

int TCPserver::SessionWriter::Write(LinuxSession* session)
{
    int retval = write(session->socket_fd, session->send_buffer.GetBufferToCHAR(), session->send_buffer.Size());
    if (retval > 0)
    {
        if (static_cast<size_t>(retval) == session->send_buffer.Size())
        {
            session->send_buffer.Clear();
        }
        else
        {
            session->send_buffer.PushFrontRange(retval);
        }
    }
    
    return retval;
}

TCPserver::TCPserver(uint16_t port): IServer(port), router(Router::GetInstance())
{}

bool TCPserver::Initialize()
{
    // socket initialize
    sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return false;
    if (listen(sock, 5) == -1) return false;

    // thread pool initialize
    recverPool = new ThreadPool;
    senderPool = new ThreadPool;
    processPool = new ThreadPool;
    dbProcessPool = new ThreadPool;
    if (recverPool == nullptr || senderPool == nullptr || processPool == nullptr || dbProcessPool == nullptr) return false;

    process = new PacketProcess;
    if (process == nullptr || process->Initialize() == false) return false;
    std::cout << "PacketProcess is Ready" << std::endl;
    dbProcess = new DBProcess;
    if (dbProcess == nullptr || dbProcess->Initialize()  == false) return false;
    std::cout << "DB Process is Ready" << std::endl;    

    // sender & recver 생성
    unsigned int core_count = 8; //std::thread::hardware_concurrency();
    router.Initialize(2 * core_count, core_count);
    std::cout << "Router is Ready" << std::endl;    

    std::unique_ptr<EPOLL_DATA_REUSEPORT> epoll_pointer = nullptr;
    std::unique_ptr<SessionReader> reader = nullptr;
    std::unique_ptr<SessionWriter> writer = nullptr; 
    std::unique_ptr<PacketProcessThreadElement> worker = nullptr;
    std::unique_ptr<DBProcessThreadElement> dbWorker = nullptr;
    try
    {
        for (int i = 0; i < core_count; i++)
        {
            epoll_pointer = std::make_unique<EPOLL_DATA_REUSEPORT>(true);
            if (epoll_pointer == nullptr || epoll_pointer->Initialize(sock)  == false) 
            {
                throw false;
            }
            else 
            {
                std::cout << "EPOLL " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            reader = std::make_unique<SessionReader>(i, epoll_pointer.get());
            if (reader == nullptr || reader->Initialize() == false)
            {
                throw false;
            }
            else
            {
                std::cout << "Reader " << i << " is Ready" << std::endl;    
                //sleep(1);
            }

            writer = std::make_unique<SessionWriter>(i, epoll_pointer.get()); 
            if (writer == nullptr || writer->Initialize() == false) 
            {
                throw false;
            }
            else 
            {
                std::cout << "Writer " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            
            dbWorker = std::make_unique<DBProcessThreadElement>(i, "tcp://127.0.0.1:6379", dbProcess);
            if (dbWorker == nullptr || dbWorker->Initialize() == false) 
            {
                throw false;
            }
            else 
            {
                std::cout << "DB worker " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            
            //std::cout << "Push " << i << " Start" << std::endl;  
            recverPool->AddElement(std::move(reader));
            senderPool->AddElement(std::move(writer));
            dbProcessPool->AddElement(std::move(dbWorker));
            epoll_pool.push_back(std::move(epoll_pointer));
            //std::cout << "Push " << i << " is Ready" << std::endl;  

            for (int j = 0; j < 2; j++)
            {
                std::unique_ptr<PacketProcessThreadElement> worker =
                     std::make_unique<PacketProcessThreadElement>(2 * i + j, process);
                if (worker == nullptr || worker->Initialize() == false) 
                {
                    throw false;
                }
                else  
                {
                    std::cout << "Packet Process worker " << 2 * i + j  << " is Ready" << std::endl;    
                    //sleep(1);
                }
            
                processPool->AddElement(std::move(worker));
            }

        }
    }
    catch(...)
    {
        return false;
    }
    
    return true;
}



LinuxServer::LinuxServer(): isRunning(true), tcp(nullptr)
{}
LinuxServer::~LinuxServer()
{
    Destroy();
}

void LinuxServer::Destroy()
{
    SAFE_FREE(tcp);
    //SAFE_FREE(redis_manager);
}


bool LinuxServer::Initialize(uint16_t port)
{   
    tcp = new TCPserver(port);
    if (tcp == nullptr || !tcp->Initialize()) return false;

    //redis_manager = new RedisManager("tcp://127.0.0.1:6000");
    //if (redis_manager == nullptr || redis_manager->Initialize()) return false;

    std::cout<< "Initialize Complete." << std::endl;
    return true;
}

void LinuxServer::Run()
{
    std::cout<< "Start Server!" << std::endl;
    //std::cout<< "You should press Ctrl + C to quit this runfile." << std::endl;
    std::string command;
    while (isRunning)
    {
        /*
        std::cout << "Server# ";
        std::cin >> command;

        if (command == "exit" || command == "quit")
        {
            std::cout << "서버 종료 명령을 수신했습니다. 안전 종료를 시작합니다..." << std::endl;
            isRunning = false;
        }
        else if (command == "status")
        {
            std::cout << "[STATUS] 현재 가동 중인 recver 스레드 수: " << recverPool->Size() << std::endl;
            std::cout << "[STATUS] 현재 가동 중인 sender 스레드 수: " << senderPool->Size() << std::endl;
            std::cout << "[STATUS] 현재 가동 중인 process worker 스레드 수: " << processPool->Size() << std::endl;
        }
        else
        {
            std::cout << "알 수 없는 명령입니다. (status, exit 중 입력)" << std::endl;
        }
        */
        sleep(0.5);
    }


    std::cout << "Close this server."<< std::endl;
}
