#ifndef PIPEHUB_H
#define PIPEHUB_H

#include "../Contexts/NetworkTask.hpp"
#include "../Utils/Thread/ThreadSafeContainor.hpp"
#include "../Utils/Containor/Containors.hpp"


enum PipeType : uint8_t
{
    ProcessInput,  // 작업이 필요한 element
    SendThis,      // 작업이 끝난 결과 element
    DBInput
};

using SessionPipe = ThreadSafeQueue<NetworkTask>;

class Router
{
    bool isInitialized;
    PoolUsingKey<SessionPipe> GOTOprocessWorker;
    PoolUsingKey<SessionPipe> GOTOsender;
    PoolUsingKey<SessionPipe> GOTODatabaseWorker;

    uint32_t bitmask;
    SessionPipe* GetPipe(const PipeType& ID, const uint32_t& shardKey)
    {
        SessionPipe* pipe = nullptr;
        switch(ID)
        {
        case PipeType::ProcessInput:
            pipe = GOTOprocessWorker.GetElement(shardKey);
            break;
        case PipeType::SendThis:
            pipe = GOTOsender.GetElement(shardKey);
            break;
        case PipeType::DBInput:
            pipe = GOTODatabaseWorker.GetElement(shardKey);
            break;
        }
        return pipe;
    }

public:
    Router() // : GOTOprocessWorker(100), GOTOsender(100), GOTODatabaseWorker(100) 
    {}

    ~Router() 
    {}

    // int howManyShard(second param)의 경우 2^n로 크기를 잡으시오.
    bool Initialize(uint32_t processThreadCount = 16, uint32_t senderCount = 8); 

    bool EnqueueElement(const PipeType ID, const uint32_t shardKey, NetworkTask&& session);
    bool DequeueElementAsChunk(const PipeType ID, const uint32_t shardKey, std::vector<NetworkTask>& chunk, const size_t maxSize = 32);
};


#endif