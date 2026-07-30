#ifndef AOIWORKER_H
#define AOIWORKER_H

#include "../Entity/Room.hpp"
#include "../../Core/Utils/Thread/ThreadPool.hpp"
#include "../ServerLinker/PacketFactory.hpp"
#include "../ServerLinker/AOIPipe.hpp"

class AOIEventWorker : public BasicThreadPoolElement
{
    PacketFactory factory;
    AOIPipePool& pipePool;
    const uint64_t GetTick() 
    { 
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
    void Work() override;
public:
    AOIEventWorker(uint32_t shardID, Context& context, AOIPipePool& pipePool): 
        BasicThreadPoolElement(shardID),
        factory(context),
        pipePool(pipePool)
    {}
    ~AOIEventWorker() {}
    bool Initialize()
    {
        return BasicThreadPoolElement::Initialize();
    }    
};

#endif