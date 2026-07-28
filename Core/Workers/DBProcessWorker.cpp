#include "DBProcessWorker.hpp"


void DBProcessWorker::Work()
{
    std::vector<NetElement> dbList;

    Router& router = context.general.router;
    while (isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::DBInput, shardID, dbList, 90)) continue;
        for (auto& element : dbList)
        {
            PacketResult result = PacketResult::CALL_NULL_METHOD;
            if (element.nextStage == ElementStage::Database)
            {
                element.RecordDBStartTime();
                result = dispatcher->Dispatch(element, context);
                element.RecordDBEndTime();
            }
            element.pk->SetResult(result);            
            router.EnqueueElement(PipeType::SendThis, shardID, std::move(element));
        }

        dbList.clear();
    }
}
