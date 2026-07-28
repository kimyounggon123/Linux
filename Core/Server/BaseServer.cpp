#include "BaseServer.hpp"

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


uint32_t BaseServer::maxThreadPoolCount = 8;
BaseServer::~BaseServer()
{
    processPool.FreeAllThread();
    std::cout << "[Process Pool] close this." << std::endl;
    dbProcessPool.FreeAllThread();
    std::cout << "[DB Pool] close this." << std::endl;
    senderPool.FreeAllThread();
    std::cout << "[Send Pool] close this." << std::endl;
    recverPool.FreeAllThread();
    std::cout << "[Recv Pool] close this." << std::endl;
    epoll_pool.clear();
    if (sock != -1) close(sock);
    std::cout << "Base Server Destructor" << std::endl;
}


bool BaseServer::Initialize()
{
    // socket initialize
    sock = useUDP ? socket(AF_INET, SOCK_DGRAM, 0) : socket(AF_INET, SOCK_STREAM, 0);
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
    if (process.Initialize() == false) return false;
    std::cout << "PacketProcess is Ready" << std::endl;
    if (dbProcess.Initialize()  == false) return false;
    std::cout << "DB Process is Ready" << std::endl;    

    // sender & recver 생성
    context.packetPool.Initialize();
    context.router.Initialize(maxThreadPoolCount * 2, maxThreadPoolCount);
    context.sessionManager.Initialize();

    std::cout << "Router is Ready" << std::endl;    

    std::unique_ptr<EPOLL_DATA_REUSEPORT> epoll_pointer = nullptr;
    // std::unique_ptr<SessionReader> reader = nullptr;
    // std::unique_ptr<SessionWriter> writer = nullptr; 
    std::unique_ptr<PacketProcessWorker> worker = nullptr;
    std::unique_ptr<DBProcessWorker> dbWorker = nullptr;

    try
    {
        for (int i = 0; i < maxThreadPoolCount; i++)
        {
            epoll_pointer = std::make_unique<EPOLL_DATA_REUSEPORT>(true);
            if (epoll_pointer == nullptr || epoll_pointer->Initialize(sock)  == false) 
            {
                throw false;
            }
            else 
            {
                //std::cout << "EPOLL " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            // reader = std::make_unique<SessionReader>(threadID++, epoll_pointer.get(), context);
            // if (reader == nullptr || reader->Initialize() == false)
            // {
            //     throw false;
            // }
            // else
            // {
            //     //std::cout << "Reader " << i << " is Ready" << std::endl;    
            //     //sleep(1);
            // }

            // writer = std::make_unique<SessionWriter>(threadID++, epoll_pointer.get(), context); 
            // if (writer == nullptr || writer->Initialize() == false) 
            // {
            //     throw false;
            // }
            // else 
            // {
            //     //std::cout << "Writer " << i << " is Ready" << std::endl;    
            //     //sleep(1);
            // }
            
            dbWorker = std::make_unique<DBProcessWorker>(&dbProcess, context, i, "tcp://127.0.0.1:6379");
            if (dbWorker == nullptr || dbWorker->Initialize() == false) 
            {
                throw false;
            }
            else 
            {
                //std::cout << "DB worker " << i << " is Ready" << std::endl;    
                //sleep(1);
            }
            
            //std::cout << "Push " << i << " Start" << std::endl;  
            // recverPool.AddElement(std::move(reader));
            // senderPool.AddElement(std::move(writer));
            dbProcessPool.AddElement(std::move(dbWorker));
            epoll_pool.push_back(std::move(epoll_pointer));
            //std::cout << "Push " << i << " is Ready" << std::endl;  

            for (int j = 0; j < 2; j++)
            {
                worker = std::make_unique<PacketProcessWorker>(context, &process, 2 * i + j);
                if (worker == nullptr || worker->Initialize() == false) 
                {
                    throw false;
                }
                else  
                {
                    //std::cout << "Packet Process worker " << 2 * i + j  << " is Ready" << std::endl;    
                    //sleep(1);
                }
            
                processPool.AddElement(std::move(worker));
            }

        }
    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool ServerAgent::Initialize()
{   
    if (server == nullptr) server = std::make_unique<BaseServer>(false, 8080);
    if (!server->Initialize()) return false;
    std::cout<< "Initialize Complete." << std::endl;
    return true;
}

void ServerAgent::Run()
{
    std::cout<< "=============Start Server!=============" << std::endl;
    //std::cout<< "You should press Ctrl + C to quit this runfile." << std::endl;
    std::string command;
    while (isRunning)
    {
        std::cout << "Server# ";
        std::cin >> command;

        if (command == "exit" || command == "quit")
        {
            std::cout << "서버 종료 명령을 수신했습니다. 안전 종료를 시작합니다..." << std::endl;
            isRunning = false;
        }
        else
        {
            std::cout << "알 수 없는 명령입니다. (status, exit 중 입력)" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Close this server."<< std::endl;
}