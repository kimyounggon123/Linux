#ifndef DBCOMPONENT_H
#define DBCOMPONENT_H


#include "../Core/Utils/Thread/ThreadPool.hpp"
#include "../Core/CoreServices.hpp"


#include "ServerLinker/DBPipePool.hpp"
#include "ServerLinker/DBPacketFactory.hpp"

#include "Dispatcher/DBProcessDispatcher.hpp"
#include "DBProcessWorker.hpp"


// db pipe pool의 경우 가장 먼저 생성 후 가장 마지막에 삭제시키도록 설정해야 함.
class DBComponent
{
    uint32_t threadCount;

    DBPipePool dbConnection;   // 가장 먼저 생성 / 마지막 삭제

    DBPacketFactory factory;
    DBProcessDispatcher dispatcher;
    DBProcessWorker::WorkerElements element;
    ThreadPool processPool;   

public:
    DBComponent(CoreServices& core, const uint32_t threadCount):  
        threadCount(threadCount),
        dbConnection(threadCount, 200),
        factory(core.pkPool), dispatcher(), 
        element(core, dispatcher, factory, dbConnection)
    {
    }
    ~DBComponent() 
    {}

    void Initialize();
    bool MakeWorkers(const std::string& addr = "tcp://127.0.0.1:6379");
    void Start() {processPool.Start("DB");}
    void Stop() {processPool.Stop("DB");}
    DBPipePool* GetConnection() {return &dbConnection;}
};

#endif