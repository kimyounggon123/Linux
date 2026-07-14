#include "DBProcessThreadElement.hpp"

void DBProcessThreadElement::Work()
{
    if (process == nullptr) return;
    std::vector<DBProcessElement> elementList;
    while (isRunning)
    {
        if (!router.DequeueDBElementAsChunk(elementList)) continue;

        for (auto& element : elementList)
        {
            element.redis = &redis;
            PROCESS_RESULT result = process->Dispatch(element);
        
            if (result == PROCESS_RESULT::NULL_METHOD)
            {
                printf("Call null method. Packet Type: %d\n", element.pk->GetTypeUINT());
            }
            element.pk->SetResult(
                result == PROCESS_RESULT::SUCCESS 
                ? PacketResult::DatabaseSuccess : PacketResult::DatabaseFail);            

            NetElement retval = {element.addr, element.session, element.pk};
            router.EnqueueElement(PipeType::ProcessOutput, std::move(retval));
        }
       
        elementList.clear();
    }
}

void DBProcessThreadElement::WriteRedis()
{
    // std::string tokenSTR;
    // std::string tcpIDSTR;
    // while (isRunning)
    // {
    //     DBBasicElement element;
    //     if (!router.DequeueDBProcess(element, ID)) continue;
    //     //if (element.session == nullptr || element.session->udp_token == 0) continue;
    //     tokenSTR = redis.IntToString(element.token);
    //     tcpIDSTR = redis.IntToString(element.sessionID);
    //     if (redis.Exist(tokenSTR)) continue;
    //     redis.Set(tokenSTR, tcpIDSTR);
    // }   
}

void DBProcessThreadElement::Destroy()
{
    redis.flushall();
    BasicThreadPoolElement::Destroy();
}
