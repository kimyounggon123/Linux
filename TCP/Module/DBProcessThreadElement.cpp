#include "DBProcessThreadElement.hpp"

void DBProcessThreadElement::Work()
{
    if (process == nullptr) return;

    while (isRunning)
    {
        DBBasicElement element;
        if (!router.DequeueDBProcess(element, ID)) continue;
        element.session->AddRef();

        element.redis = &redis;
        auto func = process->GetFunc(element.processID);

        func(element);

        element.session->Release();
    }
}

void DBProcessThreadElement::Destroy()
{
    redis.flushall();
    BasicThreadPoolElement::Destroy();
}
