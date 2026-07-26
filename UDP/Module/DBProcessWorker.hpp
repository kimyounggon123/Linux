#ifndef DATABASEPROCESSPOOLELEMENT_H
#define DATABASEPROCESSPOOLELEMENT_H

#include "UtilsLinker.hpp"
#include "../../Public/Router.hpp"
#include "DBProcess.hpp"

class DBProcessWorker : public BasicThreadPoolElement
{
    RedisControl redis;
    DBContext context;

    DBProcessDispatcher* process;
    void Work() override;
    void Destroy() override;

    void WriteRedis();
    //void ReadRedis();
public:
    DBProcessWorker(DBProcessDispatcher* process, BasicContext& context, uint32_t ID = 0, const char* ipport = "tcp://127.0.0.1:6000"):
        BasicThreadPoolElement(ID),
        redis(ipport), context(context, redis),
        process(process)
    {}
    ~DBProcessWorker() 
    {}

    bool Initialize() override;   
};

#endif