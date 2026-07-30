#include "AOIWorker.hpp"


void AOIEventWorker::Work()
{
    std::vector<GameTask> tasks;   
    Player* player = nullptr;
    Room* room = nullptr;
    while (isRunning)
    {
        if (!pipePool.PopGameTaskChunk(shardID, tasks, 32)) continue;
        for (auto& task : tasks)
        {
            if (task.player == nullptr)
                continue;

            if (task.room)
            {
                task.room->Update(task);
            }
            else
            {
                //task.player->Update(task);
            }

            factory.Send(task);
        }
        tasks.clear();     
        
        uint32_t tick = GetTick();

    }
}
