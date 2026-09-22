#ifndef AOITASK_H
#define AOITASK_H


// #include "Object/Player.hpp"
// #include "Object/Room.hpp"
#include "Object/ObjectManager.hpp"
#include "../../../Core/Utils/Containor/ThreadSafeContainor.hpp"


enum class TaskType
{
    NewPlayer,
    LAST_DUMMY
};

enum class TaskResult
{
    Try,
    Success,


    CALL_NULL_METHOD,
    Fail,

    LAST_DUMMY
};


union TaskData
{
    Vector2Float pos; 
};

struct AOITask
{
    uint32_t sessionID; 
    uint32_t playerID;

    TaskType type;
    TaskResult result;
    TaskData data;

    AOITask(uint32_t sessionID, uint32_t playerID, TaskType type, const TaskData& data):
        sessionID(sessionID), playerID(playerID), type(type), result(TaskResult::Try), data(data)
    {} 
};

using AOITaskPool = CachePool<AOITask>;
using AOITaskPipePool = PipePool<AOITask*>;
#endif