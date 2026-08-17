#include "GamePacketFactory.hpp"

bool GamePacketFactory::Test(Packet& pk, GameTask& task)
{
    LogTool::Log("GameFactory", "Test");
    PacketResult result = PacketResult::Success;
    pk.SetType(PacketType::Test); pk.SetResult(result);
    return true;
}   

bool GamePacketFactory::Initialize() 
{
    if (isInitialized) return false;
    processes.fill(&GamePacketFactory::NULL_METHOD);
    processes[ChangeToUINT(GameTaskType::Test)] = &GamePacketFactory::Test;
    isInitialized = true;
    return true;
}