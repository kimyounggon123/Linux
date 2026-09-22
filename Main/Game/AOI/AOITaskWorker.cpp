#include "AOITaskWorker.hpp"

std::chrono::milliseconds AOITaskWorker::normalInterval = std::chrono::milliseconds(50);
void AOITaskWorker::Work()
{
    while (isRunning)
    {
        ManagerWork();
        TaskWork();
        FrameWork();
    }
}
void AOITaskWorker::ManagerWork()
{
    playerManager.CheckPlayersLifeTime();
    roomManager.DeleteQuitedPlayerInRoom(playerManager.GetQuitList());
    roomManager.DeleteEmptyRoom();
    playerManager.DeleteQuitedPlayer();
}

void AOITaskWorker::TaskWork()
{
    if (!pipePool->PopChunk(shardID, tasks, 32)) return;
    Player* player = nullptr;
    Room* room = nullptr;
    for (auto& task: tasks)
    {
        dict->Dispatch(ChangeToUINT(task->type), player, room, task->data, utils);
    }
    tasks.clear();
}

void AOITaskWorker::FrameWork()
{
    now = Clock::now();
    auto dt = now - previous;

    roomManager.Update(dt);    

    previous = now;
    nextTick += normalInterval;
    std::this_thread::sleep_until(nextTick); // 지금 시간이 nextTick이 될 때까지 sleep;
}