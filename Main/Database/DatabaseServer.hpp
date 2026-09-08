#ifndef DATABASESERVER_H
#define DATABASESERVER_H

#include "../../Core/Server/TCP/TCPServer.hpp"
#include "DatabaseDispatcher.hpp"
#include "DatabaseTaskWorker.hpp"
#include "DBAccountPool.hpp"
class DatabaseServer : public TCPServer
{
    MariaAccountPool mariaPool;
    RedisAccountPool redisPool;

    DatabaseDispatcher dispatcher;
    bool MakeTaskWorkers() override;
public:
    DatabaseServer(uint16_t port, size_t threadPoolCount): 
        TCPServer(AF_INET, true, port, threadPoolCount)
    {}
    ~DatabaseServer()
    {}
};

#endif