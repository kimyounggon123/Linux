#ifndef SIGNSERVER_H
#define SIGNSERVER_H

#include "../../Core/Server/TCP/TCPServer.hpp"
#include "SignDispatcher.hpp"
#include "SignTaskWorker.hpp"


class SignServer : public TCPServer
{
    bool MakeTaskWorkers() override;
    SignDispatcher dispatcher;
    SignUtilEx utils;
public:
    SignServer(uint16_t port)  : TCPServer(port),
        dispatcher(), utils(services.pkPool, services.sessionManager, nullptr)
    {}
    ~SignServer() = default;
};

#endif