#include "DBProcessThreadElement.hpp"

void DBProcessThreadElement::Work()
{
    if (process == nullptr) return;

    while (isRunning)
    {
        DBBasicElement element;
        if (!router.DequeueDBProcess(element, ID)) continue;

        element.redis = &redis;
        auto func = process->GetFunc(element.processID);

        func(element);
    }
    //WriteRedis();
    //ReadRedis();
}

void DBProcessThreadElement::WriteRedis()
{
    std::string tokenSTR;
    std::string tcpIDSTR;
    while (isRunning)
    {
        DBBasicElement element;
        if (!router.DequeueDBProcess(element, ID)) continue;
        //if (element.session == nullptr || element.session->udp_token == 0) continue;
        tokenSTR = redis.IntToString(element.token);
        tcpIDSTR = redis.IntToString(element.sessionID);
        if (redis.Exist(tokenSTR)) continue;
        redis.Set(tokenSTR, tcpIDSTR);
    }   
}

void DBProcessThreadElement::Destroy()
{
    redis.flushall();
    BasicThreadPoolElement::Destroy();
}
