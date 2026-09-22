#include "AOIWorkDict.hpp"



bool AOIWorkDict::Initialize()
{
    if (isInitialized) return false;


    isInitialized = true;
    return true;
}

TaskResult AOIWorkDict::Dispatch(uint32_t taskID, Player* player, Room* room, TaskData& data, AOIWorkDictUtils& utils)
{
    if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(player, room, data, utils);
    return (this->*handlers[taskID])(player, room, data, utils);
}   
