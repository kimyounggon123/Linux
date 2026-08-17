#include "DBProcessDispatcher.hpp"


DatabaseTaskResult DBProcessDispatcher::Test(DatabaseTask& task, DBContext& context)
{
    LogTool::Log("DBDispatcher", "Test");
    return DatabaseTaskResult::Success;
}


bool DBProcessDispatcher::Initialize()
{
    if (isInitialized) return false;
    handlers[ChangeToUINT(DatabaseTaskType::Test)] = &DBProcessDispatcher::Test;
    isInitialized = true;
    return true;
}
