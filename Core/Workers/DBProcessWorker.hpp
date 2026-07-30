#ifndef DATABASEPROCESSPOOLELEMENT_H
#define DATABASEPROCESSPOOLELEMENT_H



#include "../Utils/Thread/ThreadPool.hpp"
#include "../Contexts/Contexts.hpp"
#include "../Contexts/NetworkTask.hpp"
#include "../Dispatchers/DBProcessDispatcher.hpp"

class DBProcessWorker : public BasicThreadPoolElement
{
    RedisController redis;
    DBContext context;
    DBProcessDispatcher* dispatcher;
    void Work() override;

    void WriteRedis();
    //void ReadRedis();
public:
    DBProcessWorker(DBProcessDispatcher* dispatcher, Context& context, uint32_t ID = 0, const char* ipport = "tcp://127.0.0.1:6000"):
        BasicThreadPoolElement(ID),
        redis(ipport), context(context, redis),
        dispatcher(dispatcher)
    {}
    ~DBProcessWorker() 
    {}

    bool Initialize() override
    {
        if (dispatcher == nullptr) return false;
        return BasicThreadPoolElement::Initialize();
    }   
};

#endif