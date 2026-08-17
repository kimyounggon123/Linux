#ifndef DBPROCESSDISPATCHER_H
#define DBPROCESSDISPATCHER_H


#include <functional>
#include <iostream>

#include "../Tool/RedisController.hpp"
#include "DatabaseTask.hpp"

#include "../../Core/Utils/Log.hpp"
struct DBContext
{
    RedisController& redis;
    DBContext(RedisController& redis): redis(redis) {}
    ~DBContext() = default;  
};

class DBProcessDispatcher
{

public:
    using Handler = DatabaseTaskResult(DBProcessDispatcher::*)(DatabaseTask&, DBContext&);

private:
    static constexpr uint32_t MaxSize = ChangeToUINT(DatabaseTaskType::LAST_DUMMY);
    bool isInitialized;
    std::array<Handler, MaxSize> handlers;

    DatabaseTaskResult NULL_METHOD(DatabaseTask& param, DBContext& context) { return DatabaseTaskResult::CALL_NULL_METHOD; }
    DatabaseTaskResult Test(DatabaseTask& task, DBContext& context);

public:
    DBProcessDispatcher(): isInitialized(false)
    {
        handlers.fill(&DBProcessDispatcher::NULL_METHOD);
    }
    ~DBProcessDispatcher() {}
    bool Initialize();
    DatabaseTaskResult Dispatch(uint32_t taskID, DatabaseTask& task, DBContext& context)
    {
        if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(task, context);
        return (this->*handlers[taskID])(task, context);
    }
};
#endif