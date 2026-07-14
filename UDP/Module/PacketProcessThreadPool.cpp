#include "PacketProcessThreadPool.hpp"

void PacketProcessThreadElement::Work()
{
    std::vector<NetElement> elementList;

    while(isRunning)
    {
        if (!router.DequeueElementAsChunk(PipeType::ProcessInput, elementList)) 
        {   
            continue;
        }

        for (auto& element : elementList)
        {
            PROCESS_RESULT result = process->Dispatch(element);
            switch (result)
            {
                case PROCESS_RESULT::SUCCESS:
                    element.pk->SetResult(PacketResult::Success);
                    break;

                case PROCESS_RESULT::GOTO_DB_PROCESS_THREAD:
                    EnterElementToDBProcess(element);
                    break;
                
                case PROCESS_RESULT::SUCCESS_DB_PROCESS:
                    element.pk->SetResult(PacketResult::DatabaseSuccess);
                    break;

                case PROCESS_RESULT::FAIL_DB_PROCESS:
                    element.pk->SetResult(PacketResult::DatabaseFail);
                    break;
                    
                default:
                    element.pk->SetResult(PacketResult::Fail);
                    break;
            }
            router.EnqueueElement(PipeType::ProcessOutput, std::move(element));
        }

        //std::cout << "Go to Write part!" << std::endl;
    }
}


void PacketProcessThreadElement::EnterElementToDBProcess(NetElement& element)
{
    element.pk->SetResult(PacketResult::TryDatabase);
    Packet* goToDB;
    if (router.PopPacket(goToDB))  // 패킷 받아옴
    {
        goToDB->CopyOther(element.pk); // 복사
        DBProcessElement dbElement = {element.addr, element.session, goToDB}; // DB 전용 element 생성
        router.EnqueueDBElement(std::move(dbElement)); // input
    }
    else element.pk->SetResult(PacketResult::DatabaseFail); // 패킷이 없으면
}

bool PacketProcessThreadElement::Initialize()
{
    if (process == nullptr) return false;
    return BasicThreadPoolElement::Initialize();
}
void PacketProcessThreadElement::Destroy()
{
    BasicThreadPoolElement::Destroy();
}