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


    // non-blocking socket setting
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    //ioctl(sock, FIONBIO, &mode);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return false;
    if (listen(sock, 5) == -1) return false;


    std::unique_ptr<EPOLL_DATA_REUSEPORT> epoll_pointer = nullptr;
    std::unique_ptr<PacketProcessWorker> worker = nullptr;
    try
    {
        pkPool.Initialize();
        managers.AddManager(&sessionManager);
        dispatcher.Initialize();
        db.Initialize();
        aoi.Initiailze();
        
        if (!db.MakeWorkers()) 
        {
            std::cout << "[DB] Fail making workers." << std::endl;
            throw false;
        }
        if (!aoi.MakeWorkers()) 
        {
            std::cout << "[AOI] Fail making workers." << std::endl;
            throw false;
        }
        for (int i = 0; i < threadPoolCount; i++)
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

            //dbProcessPool.AddElement(std::move(dbWorker));
            epoll_pool.push_back(std::move(epoll_pointer));
            //std::cout << "Push " << i << " is Ready" << std::endl;  

            for (int j = 0; j < 2; j++)
            {
                worker = std::make_unique<PacketProcessWorker>(services, dispatcher, 2 * i + j);
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
    server->Start();
    //std::cout<< "You should press Ctrl + C to quit this runfile." << std::endl;
    
    std::string command;
    while (isRunning)
    {
        std::cout << "Server# ";
        std::cin >> command;

        if (command == "exit" || command == "quit")
        {
            std::cout << "Start Closing server..." << std::endl;
            isRunning = false;
        }
        else
        {
            std::cout << "Undefined command. (Type status, exit)" << std::endl;
        }
        ThreadUtil::SleepMs(1000);
    }

}