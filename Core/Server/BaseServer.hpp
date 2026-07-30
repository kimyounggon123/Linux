#ifndef ISERVER_H
#define ISERVER_H

// Basic header
#include <iostream>
#include <vector>
#include <cstring>

#include "../Pipe/Router.hpp"
#include "../Packet/PacketPool.hpp"
#include "../Sessions/SessionManager.hpp"

#include "../Dispatchers/ProcessDispatcher.hpp"
#include "../Dispatchers/DBProcessDispatcher.hpp"

#include "../Workers/PacketProcessWorker.hpp"
#include "../Workers/DBProcessWorker.hpp"

#include "../Contexts/Contexts.hpp"
#include "../Contexts/NetworkTask.hpp"


#define MAX_EVENTS 128 // 한 번에 처리할 최대 이벤트 개수. 루프 당 유저 수가 아니라 루프 당 패킷 처리 수이다.
struct EPOLL_DATA_REUSEPORT
{
    bool isAlive;
    bool ET_style;

    // epoll instance
    int sock;
    int epfd;
    epoll_event event;
    epoll_event events[MAX_EVENTS];

    EPOLL_DATA_REUSEPORT(bool ET_style) : isAlive(false), ET_style(ET_style), epfd(-1), sock(0) {}
    ~EPOLL_DATA_REUSEPORT()
    {
        if (epfd != -1) close(epfd);
    }

    void ChangeStyle(bool UseEdgeTrigger) { ET_style = UseEdgeTrigger; }
    bool Initialize(int serverSocket);
};

class BaseServer
{
protected:
    bool useUDP;
    uint16_t port;
    
    int sock;
    sockaddr_in addr;

    ThreadPool recverPool;
    ThreadPool senderPool;

    ThreadPool processPool; 
    ThreadPool dbProcessPool;

    GeneralProcessDispatcher process;
    DBProcessDispatcher dbProcess;

    std::vector<std::unique_ptr<EPOLL_DATA_REUSEPORT>> epoll_pool; 

    PacketPool pkPool;
    Router router;
    SessionManager sessionManager;

    Context context;
    
    static uint32_t maxThreadPoolCount;
    uint32_t nextThreadID;

public:
    BaseServer(bool useUDP, uint16_t port): useUDP(useUDP), port(port), sock(-1), addr{},
        pkPool(9000, INFINITE), router(), sessionManager(),
        context(pkPool, router, sessionManager), nextThreadID(0)
    {}
    virtual ~BaseServer();
    virtual bool Initialize();
};

// 서버 + input 클래스
class ServerAgent
{
    bool isRunning;

    // epoll instance
    std::unique_ptr<BaseServer> server;
public:
    ServerAgent(std::unique_ptr<BaseServer>&& ptr): isRunning(true), server(std::move(ptr)) {}
    ~ServerAgent() 
    {}

    bool Initialize();
    void Run();
};
#endif

/*
    기존 구조: 기능을 스레드 풀로 분리함
    현재 구조: 파이프라인을 스레드 풀로 분리함.
*/



/*

1) Wnds 구조:
각기 다른 작업을 하는 클래스나 메서드를 하나의 스레드 풀로 만들고,
이들끼리의 통신을 Queue 형태로 구현.

장점:
각 작업끼리는 완전히 독립되어서 무거운 로직을 실행해도 풀끼리는 영향을 끼치지 않음.
OOP 스타일 최적화
프로세스 1개로 많은 수용량을 기대할 수 있음.

단점:
lock 필수. 때문에 각 풀 당 통신 로직이 복잡함.
프로세스 1개가 매우 무거워짐.
스레드 특성 상 프로세스 내에서의 정보는 Memory sharing이 되지만
프로세스-프로세스 간 정보 교환은 결국 통신이나 파일 시스템 필요(thread 장점 감소).
context switching이 발생해 정밀한 프레임 요구 시 잡기 까다로움.
또한 context switching이 일어나면 cache가 비워지므로 결국 main memory에서 읽어와야 함(cache 비친화적)

2) Linux 구조:
세션 내에서 파이프라인 구현.
즉 세션-패킷처리-세션이 1개의 작업으로 처리.

장점:
구현이 매우 쉬움. 스레드 풀 형태처럼 lock 등의 복잡한 로직을 구현할 필요 없음.
가벼움. thread/lock 생성이 없으니 프로세스 당 메모리는 가벼운 편
확장은 프로세스 exe를 더 만들어서 해결 가능해 편함.
CPU cache 최적화. 1개의 스레드가 recv, send, process 처리까지 다 하므로
유저 및 방 데이터가 CPU 1차/2차 cache memory에 머물러 있어 단일 연산은 매우 빠름.

단점:
파이프라인 구조 상 한 군데에서 막히면 해당 스레드 다운(치명적).
무거운 로직은 도입하기 어려울 수 있음(서버 당 유저 수용량 감소).

*/