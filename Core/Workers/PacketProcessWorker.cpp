#include "PacketProcessWorker.hpp"

void PacketProcessWorker::Work()
{
    BasicSession* session = nullptr;
    PacketResult result;
    Packet* pk = nullptr;

    NetWorkPipePool& processPipePool = services.processPipePool;
    NetWorkPipePool& sendPipePool = services.sendPipePool;
    while(isRunning)
    {
        if (!processPipePool.PopChunk(shardID, elementList)) 
        {   
            continue;
        }   
        for (auto& element : elementList)
        {
            pk = element.pk;
            if (pk == nullptr) continue;

            result = PacketResult::CALL_NULL_METHOD; // base result value

            element.RecordProcessStartTime(); // 로직 시간 측정
            result = dispatcher.Dispatch(pk->GetTypeUINT(), element); // 실제 패킷 로직 처리
            element.RecordProcessEndTime(); // 로직 시간 측정


            if (element.nextStage == ElementStage::Send) // 일반적인 패킷 처리
            {
                element.pk->SetResult(result); // 패킷 결과 처리
                sendPipePool.Push(shardID, std::move(element)); // Send Pool로 전송
            }
        }
        elementList.clear();
    }
}