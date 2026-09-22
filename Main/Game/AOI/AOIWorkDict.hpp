#ifndef AOIWORKDICT_H
#define AOIWORKDICT_H

// template <typename DispatcherName, typename Utils>
// class INetworkTaskDispatcher
// {
// public:
//     using Handler = PacketResult(DispatcherName::*)(NetworkTask&, Utils&);
// private:

//     static constexpr uint32_t MaxSize = ChangeToUINT(PacketType::LAST_DUMMY);
//     std::array<Handler, MaxSize> handlers;
//     PacketResult NULL_METHOD(NetworkTask& task, Utils& utils) 
//     { 
//         task.pk->PrintInformation();
//         task.pk->ClearBuffer();
//         return PacketResult::CALL_NULL_METHOD;
//     }
// protected:

//     bool isInitialized;
//     bool EmplaceProcess(uint32_t ID, Handler handler)
//     {
//         if (ID >= MaxSize) return false;
//         handlers[ID] = handler;
//         return true;
//     }

// public:  
//     INetworkTaskDispatcher(): isInitialized(false)
//     {
//         handlers.fill(&DispatcherName::NULL_METHOD);
//     }
//     virtual ~INetworkTaskDispatcher()
//     {
//         handlers.fill(&DispatcherName::NULL_METHOD);
//     }
//     virtual bool Initialize() = 0;

//     PacketResult Dispatch(uint32_t taskID, NetworkTask& task, Utils& utils)
//     {
//         if (task.pk->GetResult() != PacketResult::Try) return task.pk->GetResult();
//         if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(task, utils);
//         return (static_cast<DispatcherName*>(this)->*handlers[taskID])(task, utils);
//     }
// };

#include "AOITask.hpp"
#include "../../../Core/Utils/utils.h"
struct AOIWorkDictUtils
{
    PlayerManager* playerManager;
    RoomManager* roomManager;

    AOIWorkDictUtils(PlayerManager* playerManager, RoomManager* roomManager):
       playerManager(playerManager), roomManager(roomManager)
    {}

    AOIWorkDictUtils(const AOIWorkDictUtils& other):
        playerManager(other.playerManager), roomManager(other.roomManager)
    {}
};

class AOIWorkDict
{
public:
    using Handler = TaskResult(AOIWorkDict::*)(Player*, Room*, TaskData&, AOIWorkDictUtils&);
private:
    static constexpr uint32_t MaxSize = ChangeToUINT(TaskType::LAST_DUMMY);
    std::array<Handler, MaxSize> handlers;
    TaskResult NULL_METHOD(Player*, Room*, TaskData&, AOIWorkDictUtils&) 
    { 
        return TaskResult::CALL_NULL_METHOD;
    }

    bool isInitialized;
    bool EmplaceProcess(uint32_t ID, Handler handler)
    {
        if (ID >= MaxSize) return false;
        handlers[ID] = handler;
        return true;
    }

public:
    AOIWorkDict(): isInitialized(false)
    {
        handlers.fill(&AOIWorkDict::NULL_METHOD);
    }
    ~AOIWorkDict()
    {
        handlers.fill(&AOIWorkDict::NULL_METHOD);
    }
    bool Initialize();
    TaskResult Dispatch(uint32_t taskID, Player* player, Room* room, TaskData& data, AOIWorkDictUtils& utils);
};

#endif