#ifndef SIGNSERVER_H
#define SIGNSERVER_H

#include "../../Core/Server/TCP/TCPServer.hpp"
#include "../../Core/Server/IPC/SocketIPC.hpp"
#include "SignDispatcher.hpp"
#include "SignTaskWorker.hpp"


class SignServer : public TCPServer
{
    NetWorkPipePool toSendDB;
    SignDispatcher dispatcher;
    TCP_IPC DBconnection;

    bool MakeTaskWorkers() override;

public:
    SignServer(bool primateAddrFlag, uint16_t port, size_t threadPoolCount)  :
        TCPServer(AF_INET, primateAddrFlag, port, threadPoolCount, true),
        dispatcher(), toSendDB(threadPoolCount, 200),
        DBconnection(AF_INET, 4000, threadPoolCount, &toSendDB, services.sendPipePool, services.pkPool) {}
    ~SignServer() = default;

    void Start() override;
    void Stop() override;
};

#endif