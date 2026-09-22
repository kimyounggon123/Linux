#ifndef SOCKETIPC_H
#define SOCKETIPC_H

#include "../Session/BasicSession.hpp"
#include "../../Utils/Containor/ThreadSafeContainor.hpp"
#include "../../Utils/Thread/ThreadPool.hpp"
#include "../CoreServices.hpp"

class SocketIPC
{
protected:
    int domain;

    ProtocolType protocol;
    uint16_t port;
    int sock;
    sockaddr_in serverAddr;  

    uint32_t threadCount;
    ThreadPool recvPool;
    ThreadPool sendPool;
    NetWorkPipePool* requestPool;
    NetWorkPipePool* responsePool;

    PacketPool* pkPool;

    
public:
    SocketIPC(int domain, ProtocolType protocol, uint16_t port,
        uint32_t threadCount, NetWorkPipePool* requestPool, NetWorkPipePool* responsePool, PacketPool* pkPool): 
        domain(domain), protocol(protocol), port(port), sock(-1), serverAddr{},
        threadCount(threadCount), requestPool(requestPool), responsePool(responsePool), pkPool(pkPool) {}
    
    SocketIPC(const SocketIPC&) = delete;
    SocketIPC& operator=(const SocketIPC&) = delete;

    virtual ~SocketIPC()
    {
        Stop();
    }

    bool MakeSocket(const char* addr);    
    virtual bool MakeWorkers() = 0; 
    void Start();
    void Stop();
};


class TCP_IPC : public SocketIPC
{
    class Recver : public BasicThreadPoolElement
    {
        int& sock;
        PacketPool* pkPool;
        NetWorkPipePool* responsePool;
        RecvBuffer buffer;
        void Work() override;
        void MakePacketFromBuffer();
    public:
        Recver(uint32_t shardID, int& sock, NetWorkPipePool* responsePool, PacketPool* pkPool): 
            BasicThreadPoolElement(shardID),
            sock(sock), responsePool(responsePool), pkPool(pkPool)
        {}
    };
    class Sender : public BasicThreadPoolElement
    {
        int& sock;
        std::vector<NetworkTask> tasks;
        PacketPool* pkPool;
        NetWorkPipePool* requestPool;
        SendBuffer buffer;
        void Work() override;
    public:
        Sender(uint32_t shardID, int& sock, NetWorkPipePool* requestPool, PacketPool* pkPool): 
            BasicThreadPoolElement(shardID),
            sock(sock), requestPool(requestPool), pkPool(pkPool)
        {}
    };

public:
    TCP_IPC(int domain, uint16_t port,
        uint32_t threadCount, NetWorkPipePool* requestPool, NetWorkPipePool* responsePool, PacketPool* pkPool):
        SocketIPC(domain, ProtocolType::TCP, port, threadCount, requestPool, responsePool, pkPool) {}

    bool MakeWorkers() override;
};


#endif