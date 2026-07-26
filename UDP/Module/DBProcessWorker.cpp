#include "DBProcessWorker.hpp"

bool DBProcessWorker::Initialize()
{
    if (process == nullptr) return false;
    return BasicThreadPoolElement::Initialize();
}

void DBProcessWorker::Work()
{
    std::vector<NetElement> dbList;

    Router& router = context.general.router;
    while (isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::DBInput, dbList, 90)) continue;
        for (auto& element : dbList)
        {
            PacketResult result = PacketResult::CALL_NULL_METHOD;
            if (element.nextStage == ElementStage::Database)
            {
                element.RecordDBStartTime();
                result = process->Dispatch(element, context);
                element.RecordDBEndTime();
            }
            element.pk->SetResult(result);            
            router.EnqueueElement(PipeType::ProcessOutput, std::move(element));
        }

        dbList.clear();
    }
}

void DBProcessWorker::Destroy()
{
    redis.flushall();
    BasicThreadPoolElement::Destroy();
}
