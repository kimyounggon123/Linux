#ifndef PACKETPROCESSTHREADPOOL_H
#define PACKETPROCESSTHREADPOOL_H

#include "../../Public/Utils/Thread/ThreadPool.hpp"
#include "PacketProcess.hpp"

class PacketProcessWorker : public BasicThreadPoolElement
{
    BasicContext& context;
    PacketProcessDispatcher* process;
    void Work() override;

    void ProcessChunkStyle();

    void EnterElementToDBProcess(NetElement& element);
public:
    PacketProcessWorker(BasicContext& context, PacketProcessDispatcher* process, uint32_t ID = 0):
        BasicThreadPoolElement(ID),
        process(process),
        context(context)
    {}

    ~PacketProcessWorker()
    {}


    bool Initialize() override;
    void Destroy() override;

};





#endif