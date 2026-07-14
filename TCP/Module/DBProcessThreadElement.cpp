#include "DBProcessThreadElement.hpp"

void DBProcessThreadElement::Work()
{
    if (process == nullptr) return;
    std::vector<NetElement> dbList;

    while (isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::DBInput, dbList, 90)) continue;
        for (auto& element : dbList)
        {
           
            
            element.context.redis = &redis;

            PacketResult result = PacketResult::CALL_NULL_METHOD;

            if (element.nextStage == ElementStage::Database)
            {
                element.RecordDBStartTime();
                result = process->Dispatch(element);
                element.RecordDBEndTime();
            }
            element.pk->SetResult(result);            
            router.EnqueueElement(PipeType::ProcessOutput, std::move(element));
        }

        dbList.clear();
    }
}

void DBProcessThreadElement::Destroy()
{
    redis.flushall();
    BasicThreadPoolElement::Destroy();
}
