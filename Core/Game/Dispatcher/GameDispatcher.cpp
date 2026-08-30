#include "GameDispatcher.hpp"
GameTaskResult GameDispatcher::Move(GameTask& task, GameContext& ctx)
{
    if (task.type != GameTaskType::Move) return GameTaskResult::Fail; 
    //LogTool::Log("GameDispatcher", "Move");
    return GameTaskResult::Success | GameTaskResult::Broadcast;
}

bool GameDispatcher::Initialize()
{
    if (isInitialized) return false;
    handlers[ChangeToUINT(GameTaskType::Test)] = &GameDispatcher::Move; 
    isInitialized = true;
    return true;
}
