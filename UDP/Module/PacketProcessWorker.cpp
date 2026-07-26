#include "PacketProcessWorker.hpp"


bool PacketProcessWorker::Initialize()
{
    if (process == nullptr) return false;
    return BasicThreadPoolElement::Initialize();
}
void PacketProcessWorker::Destroy()
{
    PacketProcessWorker::Destroy();
}

void PacketProcessWorker::Work()
{
    BasicSession* session = nullptr;
    std::vector<NetElement> elementList;
    PacketResult result;
    Packet* pk = nullptr;

    while(isRunning)
    {
        if (!context.router.DequeueElementAsChunk(PipeType::ProcessInput, elementList)) 
        {   
            continue;
        }
        
        for (auto& element : elementList)
        {
            pk = element.pk;
            if (pk == nullptr) continue;

            result = PacketResult::CALL_NULL_METHOD; // base result value
            if (element.nextStage == ElementStage::GeneralProcess) // 디버깅용. 만약 다른 서버로 가야할 패킷이 올 경우를 처리하는 코드
            {
                element.RecordProcessStartTime(); // 로직 시간 측정
                result = process->Dispatch(element, context); // 실제 패킷 로직 처리
                element.RecordProcessEndTime(); // 로직 시간 측정
            }
            
            if (element.nextStage == ElementStage::Database) // DB 작업을 요구하는 경우
            {
                EnterElementToDBProcess(element);
            }
            else // 일반적인 패킷 처리
            {
                element.pk->SetResult(result); // 패킷 결과 처리
                context.router.EnqueueElement(PipeType::ProcessOutput, std::move(element)); // Send Pool로 전송
            }
        }
        elementList.clear();
    }
}

void PacketProcessWorker::ProcessChunkStyle()
{
    /*
    LinuxSession* session = nullptr;
    std::vector<NetElement> elementList;
    PacketResult result;
    Packet* pk = nullptr;

    while(isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::ProcessInput, elementList)) 
        {   
            continue;
        }
        
        for (auto& element : elementList)
        {
            pk = element.pk;
            if (pk == nullptr) continue;

            result = PacketResult::CALL_NULL_METHOD; // base result value
            if (element.nextStage == ElementStage::GeneralProcess) // 디버깅용. 만약 다른 서버로 가야할 패킷이 올 경우를 처리하는 코드
            {
                element.RecordProcessStartTime(); // 로직 시간 측정
                result = process->Dispatch(element); // 실제 패킷 로직 처리
                element.RecordProcessEndTime(); // 로직 시간 측정
            }
            
            if (element.nextStage == ElementStage::Database) // DB 작업을 요구하는 경우
            {
                EnterElementToDBProcess(element);
            }
            else // 일반적인 패킷 처리
            {
                element.pk->SetResult(result); // 패킷 결과 처리
                router.EnqueueElement(PipeType::ProcessOutput, std::move(element)); // Send Pool로 전송
            }
        }
        elementList.clear();
    }
    */
}


void PacketProcessWorker::EnterElementToDBProcess(NetElement& element)
{
    NetElement dbElement = element; // DB 전용 element 생성
    context.router.EnqueueElement(PipeType::DBInput, std::move(dbElement)); // input
}
