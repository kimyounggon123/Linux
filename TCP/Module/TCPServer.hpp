#ifndef LINUXSERVER_H
#define LINUXSERVER_H


#include "../../Core/BaseServer.hpp"
#include "../../Database/DBComponent.hpp"
#include "../../Game/AOIComponent.hpp"
#include "TCPSession.hpp"

/*
    현재 풀 / 리소스 사용 클래스
    Thread Pool
    Packet Pool
    Session Manager
*/

class TCPServer : public BaseServer
{
    class SessionReader : public BasicThreadPoolElement
    {
        EPOLL_DATA_REUSEPORT* epoll_data;
        CoreServices& services;

        // while loop method    
        void Work() override;

        bool ReadLogic(TCPSession* session);
        void HelloNewSession();
        void ByeSession(TCPSession* session);
        void ProcessClientBuffer(TCPSession* session);
    public:
        SessionReader(uint32_t ID, EPOLL_DATA_REUSEPORT* got_epoll, CoreServices& services):
            BasicThreadPoolElement(ID),
            epoll_data(got_epoll), services(services)
        {}
        ~SessionReader() {}
    };

    class SessionWriter : public BasicThreadPoolElement
    {
        EPOLL_DATA_REUSEPORT* epoll_data;
        CoreServices& services;

        static int maxSendCount;
        void Work() override;
        int Write(TCPSession* session);

    public:
        SessionWriter(uint32_t ID, EPOLL_DATA_REUSEPORT* got_epoll,  CoreServices& services): 
            BasicThreadPoolElement(ID),
            epoll_data(got_epoll), services(services)
        {}
        ~SessionWriter() {}
    };

public:
    TCPServer(uint16_t port): BaseServer(false, port) {}
    ~TCPServer() {}
    bool Initialize() override;
};


// class LinuxServer
// {
//     bool isRunning;

//     // epoll instance
//     TCPserver* tcp;
    

//     void Destroy();
// public:
//     LinuxServer();
//     ~LinuxServer();

//     bool Initialize(uint16_t port);
//     void Run();
// };
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