#include "DBProcessWorker.hpp"


void DBProcessWorker::Work()
{
    Packet* pk = nullptr;
    DBContext context = {redis};
    BasicSession* session = nullptr;
    while (isRunning)
    {
        if (!elements.dbPipePool.PopChunk(shardID, dbList, 90)) continue;
        for (auto& task : dbList)
        {
            task.result = elements.dispatcher.Dispatch(ChangeToUINT(task.type), task, context);
            if (task.ignoreThis) continue;           

            //elements.Print("Work");
            pk = elements.factory.MakePacket(task);
            session = elements.core.sessionManager.FindSession(task.sessionID);
            if (session == nullptr) continue;

            NetworkTask toSend = {ElementStage::Send, session, pk};            
            elements.core.sendPipePool.Push(shardID, std::move(toSend));
        }
        dbList.clear();
    }
}
