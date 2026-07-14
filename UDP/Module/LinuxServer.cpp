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
    recverPool(nullptr), senderPool(nullptr), processPool(nullptr), process(nullptr)
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

    SAFE_FREE(process);
    epoll_pool.clear();

    if (sock != -1) close(sock);
}


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
    if (buffer.GetFreeSpace() < Packet::MAX_SIZE) buffer.MoveDataFront();
    socklen_t clientAddrLen = sizeof(clientAddr); 
    int str_len =  recvfrom(owner->GetSocket(), buffer.GetWritePtr(), buffer.GetFreeSpace(), 0,
            (struct sockaddr*)&clientAddr, 
            &clientAddrLen);
    if (str_len == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return false;
        else  return false;
    }
    else ProcessClientBuffer(str_len);             
    return true;
}   

void UDPserver::SessionReader::ProcessClientBuffer(int recvLength)
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

    Packet* pk = nullptr;
    size_t offset = 0;
    if (!router.PopPacket(pk)) return;

    pk->ClearBuffer();
    ERROR_CODE code = pk->Deserialize(buffer.GetReadPtr(), recvLength, offset);
        
    if (code != ERROR_CODE::SUCCESS) // 또는 !code (SUCCESS가 0인 경우)
    {
        std::cout << "Deserialize fail" << std::endl;
        if (code == ERROR_CODE::NEED_EXTRA_DATA)
        {
            std::cout << "NEED_EXTRA_DATA" << std::endl;
            // 데이터가 더 필요하므로, 남은 데이터를 버퍼 앞쪽으로 당기고 다음 recv를 기다림
            buffer.MoveDataFront();
            router.PushPacket(pk);
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
    NetElement element = {clientAddr, nullptr, pk};
    router.EnqueueElement(PipeType::ProcessInput, std::move(element));  
}

int UDPserver::SessionWriter::maxSendCount = 60;

void UDPserver::SessionWriter::Work() 
{
    std::vector<NetElement> elementList;
    while (isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::ProcessOutput, elementList)) 
        {
            //std::cout << "cannot found session in writer" << std::endl;
            continue;
        }
        // 패킷 꺼내오기
        
        for (auto it = elementList.begin(); it != elementList.end(); it++)
        {
            NetElement& element = *it;
            LinuxSession* session = element.session;
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
            router.PushPacket(pk);
        }
        elementList.clear(); // 기존 pkList는 반드시 비우기.
    }
}

int UDPserver::SessionWriter::Write(const NetElement& element)
{
    if (!element.pk->Serialize(buffer.GetVector())) return -1;
    socklen_t clientAddrLen = sizeof(element.addr); 
    int retval = sendto(owner->GetSocket(), buffer.GetBufferToCHAR(), buffer.Size(), 0,
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

UDPserver::UDPserver(uint16_t port): IServer(port), redisDBPool(nullptr), router(Router::GetInstance())
{}

bool UDPserver::Initialize()
{
    // socket initialize
    sock = socket(AF_INET, SOCK_DGRAM, 0);
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
    process = new PacketProcess;
    if (recverPool == nullptr || senderPool == nullptr || processPool == nullptr) return false;
    if (process == nullptr || process->Initialize()) return false;


    // sender & recver 생성
    unsigned int core_count = std::thread::hardware_concurrency();
    router.Initialize(9000);
    
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
            if (epoll_pointer == nullptr || epoll_pointer->Initialize(sock)) 
            {
                throw false;
            }

            reader = std::make_unique<SessionReader>(this, i, epoll_pointer.get());
            if (reader == nullptr || !reader->Initialize())
            {
                throw false;
            }

            writer = std::make_unique<SessionWriter>(this, i, epoll_pointer.get()); 
            if (writer == nullptr || !writer->Initialize()) 
            {
                throw false;
            }
            
            dbWorker = std::make_unique<DBProcessThreadElement>("tcp://127.0.0.1:6000");
            if (dbWorker == nullptr || !dbWorker->Initialize()) 
            {
                throw false;
            }

            recverPool->AddElement(std::move(reader));
            senderPool->AddElement(std::move(writer));
            redisDBPool->AddElement(std::move(dbWorker));
            epoll_pool.push_back(std::move(epoll_pointer));

            for (int j = 0; j < 2; j++)
            {
                std::unique_ptr<PacketProcessThreadElement> worker =
                     std::make_unique<PacketProcessThreadElement>(2 * i + j, process);
                if (worker == nullptr) return false;
                if (!worker->Initialize()) 
                {
                    throw false;
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



LinuxServer::LinuxServer(): isRunning(false),
    tcp(nullptr)            //, redis_manager(nullptr)
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
    tcp = new UDPserver(port);
    if (tcp == nullptr || tcp->Initialize()) return false;

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
