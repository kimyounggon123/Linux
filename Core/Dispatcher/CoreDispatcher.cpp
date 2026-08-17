#include "CoreDispatcher.hpp"


PacketResult CoreDispatcher::Test(NetworkTask& task)
{
    DatabaseTask dbTask = {DatabaseTaskType::Test, task.session->GetID()};
    GameTask gameTask = {GameTaskType::Test, 0, 0, task.session->GetID()};

    connections.toDB->Push(task.session->GetID(), std::move(dbTask));
    connections.toAOI->Push(task.session->GetID(), std::move(gameTask));

    LogTool::Log("CoreDispatcher", "Test");
    task.GoToHere(ElementStage::DropThis);
    return PacketResult::Success;
}

bool CoreDispatcher::Initialize()
{
    if (isInitialized) return false;
    handlers[ChangeToUINT(PacketType::Test)] = &CoreDispatcher::Test;
    isInitialized = true;
    return true; 
}