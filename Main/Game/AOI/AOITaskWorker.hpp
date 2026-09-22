#ifndef AOITASKWORKER_H
#define AOITASKWORKER_H

#include "../../../Core/Utils/Thread/ThreadPool.hpp"
#include "AOIWorkDict.hpp"

class AOITaskWorker : public BasicThreadPoolElement
{   
    PlayerManager playerManager;
    RoomManager roomManager;
    void ManagerWork();

    std::vector<AOITask*> tasks;
    AOITaskPool* taskPool;
    AOITaskPipePool* pipePool;
    AOIWorkDict* dict;
    AOIWorkDictUtils utils;
    void TaskWork();    

    static std::chrono::milliseconds normalInterval;    
    Clock::time_point now;
    Clock::time_point previous;
    Clock::time_point nextTick; // 이 tick에 도달할 때 까지 sleep
    void FrameWork();
    void Work() override;

public:
    AOITaskWorker(uint32_t shardID, AOITaskPool* taskPool, AOITaskPipePool* pipePool, AOIWorkDict* dict):
        BasicThreadPoolElement(shardID),
        playerManager(shardID), roomManager(shardID), 
        taskPool(taskPool), pipePool(pipePool), dict(dict), utils{&playerManager, &roomManager},
        now(Clock::now()), previous(Clock::now()), nextTick(Clock::now()) {}
    ~AOITaskWorker() {}

    PlayerManager* GetPlayerManager() {return &playerManager;}
    RoomManager* GetRoomManager() {return &roomManager;}
};

#endif