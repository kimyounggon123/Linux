#include "DBPacketFactory.hpp"

bool DBPacketFactory::Initialize()
{
    if (isInitialized) return false;

    processes.fill(&DBPacketFactory::NULL_METHOD);
    
    processes[ChangeToUINT(DatabaseTaskType::Test)] = &DBPacketFactory::Test;

    isInitialized = true;
    return true;
}

bool DBPacketFactory::Test(Packet& pk, DatabaseTask& task)
{
    LogTool::Log("DBFactory", "Test");
    PacketResult result = task.result == DatabaseTaskResult::Success ? PacketResult::Success : PacketResult::INVALID_CLIENT;
    pk.SetType(PacketType::Test); pk.SetResult(result);
    return true;
}