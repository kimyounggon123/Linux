#ifndef AOIWORKER_H
#define AOIWORKER_H

#include "Entity/Room.hpp"
#include "ServerLinker/GamePacketFactory.hpp"
#include "ServerLinker/AOIPipe.hpp"
#include "Dispatcher/GameDispatcher.hpp"


#include "../Core/Utils/Thread/ThreadPool.hpp"
#include "../Core/CoreServices.hpp"

class AOIEventWorker : public BasicThreadPoolElement
{
public:
    struct WorkerElement
    {
        CoreServices& core;
        GameDispatcher& dispatcher;
        GamePacketFactory& factory;
        AOIPipePool& aoiPipePool;
        PlayerManager& playerManager;

        WorkerElement(CoreServices& core, GameDispatcher& dispatcher,
        GamePacketFactory& factory,
        AOIPipePool& aoiPipePool,
        PlayerManager& playerManager):
        core(core),
        dispatcher(dispatcher), factory(factory),
        aoiPipePool(aoiPipePool),
        playerManager(playerManager) {}

        WorkerElement(uint32_t threadID, const WorkerElement& other):
            core(other.core),         
            dispatcher(other.dispatcher), factory(other.factory),
            aoiPipePool(other.aoiPipePool),
            playerManager(other.playerManager)
        {}

        void Print(const char* where)
        {
            std::cout << "[In]: " << where << std::endl;
            std::cout << "this         = " << this << '\n';
            std::cout << "connection   = " << &aoiPipePool << '\n';
            std::cout << "dispatcher   = " << &dispatcher << '\n';
            std::cout << "factory      = " << &factory << '\n';
        }
    };

private:
    WorkerElement element;
    RoomManager roomManager;

    const uint64_t GetTick() 
    { 
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
    void Work() override;
  
public:
    AOIEventWorker(WorkerElement& element,
        uint32_t shardID = 0): 
        BasicThreadPoolElement(shardID), roomManager(shardID), element(element)
    {}
    ~AOIEventWorker() {}
};

#endif