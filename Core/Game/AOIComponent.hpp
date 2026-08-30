#ifndef AOICOMPONENT_H
#define AOICOMPONENT_H

#include "../Core/Utils/Thread/ThreadPool.hpp"

#include "Entity/EntityManager.hpp"
#include "Dispatcher/GameDispatcher.hpp"
#include "ServerLinker/AOIPipe.hpp"
#include "ServerLinker/GamePacketFactory.hpp"
#include "AOIWorker.hpp"

class AOIComponent
{
    uint32_t threadCount;

    AOIPipePool aoiConnection;

    GameDispatcher dispatcher;
    GamePacketFactory factory;
    PlayerManager playerManager;

    AOIEventWorker::WorkerElement element;
    ThreadPool processPool;
public:
    AOIComponent(CoreServices& core, const uint32_t threadCount):
        threadCount(threadCount),
        dispatcher(), factory(*core.pkPool),
        aoiConnection(threadCount, 200),
        element(core, dispatcher, factory, aoiConnection, playerManager)
    {
        //element.Print("AOI Constructor");
    }
    ~AOIComponent()
    {}

    void Initiailze();
    bool MakeWorkers();
    void Start() {processPool.Start("AOI");}
    void Stop() {processPool.Stop("AOI");}
    AOIPipePool* GetConnection() {return &aoiConnection;}
};


#endif