#include "AOIWorker.hpp"
void AOIEventWorker::Work()
{
    Player* player = nullptr;
    Room* room = nullptr;
    Packet* pk = nullptr;
    std::vector<GameTask> tasks; 
    GameTaskResult flag = GameTaskResult::Fail;
    uint32_t tick;

    //RoomManager& roomManager = *element.roomManagerPool.GetElement(shardID);
    GameContext context = {element.playerManager, roomManager};
    
    BasicSession* session = nullptr;
    while (isRunning)
    {
        
        tick = GetTick();
        for (auto* room : roomManager.GetList())
        {
            room->TickUpdate(tick);
        }

        if (!element.aoiPipePool.PopChunk(shardID, tasks)) continue;
        for (auto& task : tasks)
        {
            flag = element.dispatcher.Dispatch(ChangeToUINT(task.type), task, context); 
            if (task.ignoreThis) continue;
            pk = element.factory.MakePacket(task);

            session = element.core.sessionManager.FindSession(task.sessionID);
            NetworkTask toSend(ElementStage::Send, session, pk);            
            element.core.sendPipePool.Push(shardID, std::move(toSend));
        }
        tasks.clear();      
    }
}
