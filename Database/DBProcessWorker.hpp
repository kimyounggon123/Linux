#ifndef DATABASEPROCESSPOOLELEMENT_H
#define DATABASEPROCESSPOOLELEMENT_H

#include "Dispatcher/DBProcessDispatcher.hpp"
#include "Tool/RedisController.hpp"
#include "ServerLinker/DBPipePool.hpp"
#include "ServerLinker/DBPacketFactory.hpp"
#include "../Core/Utils/Thread/ThreadPool.hpp"
#include "../Core/CoreServices.hpp"

class DBProcessWorker : public BasicThreadPoolElement
{
public:

    // 외부 element 확보하기
    struct WorkerElements
    {
        CoreServices& core;
        DBProcessDispatcher& dispatcher;
        DBPacketFactory& factory;    
        DBPipePool& dbPipePool;

        WorkerElements(CoreServices& core, DBProcessDispatcher& dispatcher, DBPacketFactory& factory, DBPipePool& dbPipePool) :
            core(core), dispatcher(dispatcher), factory(factory), dbPipePool(dbPipePool)
        {}
        WorkerElements(const WorkerElements& other):
            core(other.core), dispatcher(other.dispatcher), factory(other.factory), dbPipePool(other.dbPipePool)
        {}

        void Print(const char* where)
        {
            std::cout << "[In]: " << where << std::endl;
            std::cout << "this         = " << this << '\n';
            std::cout << "dbConnection = " << &dbPipePool << '\n';
            std::cout << "dispatcher   = " << &dispatcher << '\n';
            std::cout << "factory      = " << &factory << '\n';
        }
    };

private:
    WorkerElements elements;
    RedisController redis;
    std::vector<DatabaseTask> dbList;
    void Work() override;

public:
    DBProcessWorker(WorkerElements& elements,
        uint32_t ID = 0, const char* ipport = "tcp://127.0.0.1:6000"):
        BasicThreadPoolElement(ID),
        elements(elements), redis(ipport) 
    {
        //elements.Print("Worker");
        //this->elements.Print("Worker Field");
    }
    ~DBProcessWorker()  {}
};

#endif