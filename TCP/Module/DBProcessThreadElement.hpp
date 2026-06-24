#ifndef DATABASEPROCESSPOOLELEMENT_H
#define DATABASEPROCESSPOOLELEMENT_H

#include "UtilsLinker.hpp"
#include "Router.hpp"
#include "DBProcess.hpp"

class DBProcessThreadElement : public BasicThreadPoolElement
{
    RedisControl redis;
    Router& router;

    DBProcess* process;
    void Work() override;
    void Destroy() override;


    void WriteRedis();
    //void ReadRedis();
public:
    DBProcessThreadElement(uint32_t ID = 0, const char* ipport = "tcp://127.0.0.1:6000",  DBProcess* process = nullptr):
        BasicThreadPoolElement(ID),
        redis(ipport), process(process),
        router(Router::GetInstance())
    {}
    ~DBProcessThreadElement() = default;    
};

#endif