#include "BaseServer.hpp"

bool EPOLL_DATA_REUSEPORT::Initialize(int serverSocket)
{
    if (isAlive) return true;

    // epoll 인스턴스 생성
    epfd = epoll_create(1); 

    sock = serverSocket;

    
    // 서버 소켓(Listen 소켓)을 epoll에 등록
    event.events = event_style; // 데이터 수신(접속 요청)을 관찰
    if (ET_style) event.events = event.events | EPOLLET;
    
    event.data.fd = sock;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event);
    //std::cout << "ADD ret = " << ret << ", epfd = " << epfd << ", sock = " << sock << '\n';
    isAlive = true;
    return true;
}



bool BaseServer::MakeSocket()
{
    // socket initialize
    sock = useUDP ? socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) : socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        std::cerr << "socket error: "
                << strerror(errno) << '\n';
        return false;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (!useUDP) setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // non-blocking socket setting
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    //ioctl(sock, FIONBIO, &mode);


    int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) 
    {
        std::cerr << "Bind error: "
          << std::error_code(errno, std::generic_category()).message()
          << std::endl;
        return false;
    }

    if (!useUDP)
    {
        if (listen(sock, 5) == -1)
        {
            std::cerr << "Listen error: "
            << std::error_code(errno, std::generic_category()).message()
            << std::endl;
            return false;
        } 
    }

    return true;
}
bool BaseServer::MakeEPOLL()
{
    std::unique_ptr<EPOLL_DATA_REUSEPORT> epoll_pointer = nullptr;
    try
    {
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
        }
    }
    catch(...)
    {
        return false;
    }
    return true;
}

bool BaseServer::Initialize()
{
    if (!MakeSocket()) return false;
    if (!MakeEPOLL()) return false;
    if (!MakeSessionWorkers()) return false;
    if (!MakeTaskWorkers()) return false;
    return true;
}

bool ServerAgent::Initialize()
{   
    if (server == nullptr || !server->Initialize()) return false;
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