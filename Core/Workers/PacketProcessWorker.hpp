#ifndef PACKETPROCESSTHREADPOOL_H
#define PACKETPROCESSTHREADPOOL_H

#include "../Contexts/Contexts.hpp"
#include "../Contexts/NetElement.hpp"
#include "../Dispatchers/ProcessDispatcher.hpp"
#include "../Utils/Thread/ThreadPool.hpp"

class PacketProcessWorker : public BasicThreadPoolElement
{
    Context& context;
    GeneralProcessDispatcher* dispatcher;
    void Work() override;
public:
    PacketProcessWorker(Context& context, GeneralProcessDispatcher* dispatcher, uint32_t ID = 0):
        BasicThreadPoolElement(ID),
        dispatcher(dispatcher),
        context(context)
    {}

    ~PacketProcessWorker()
    {}

    bool Initialize() override
    {
        if (dispatcher == nullptr) return false;
        return BasicThreadPoolElement::Initialize();
    }
};





#endif