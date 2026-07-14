#ifndef PIPEHUB_H
#define PIPEHUB_H

#include "NetStruct.hpp"

enum PipeType : uint8_t
{
    ProcessInput = 0U,  // 작업이 필요한 element
    ProcessOutput,      // 작업이 끝난 결과 element
    DBInput
};

using SessionPipe = ThreadSafeQueue<NetElement>;
class Router
{
    bool isInitialized;
    static Router* instance;

    ThreadSafePool<Packet> PacketPool;

    SessionPipe GOTOprocessWorker;
    SessionPipe GOTOsender;
    SessionPipe GOTODatabaseWorker;

    Router(): PacketPool(100), GOTOprocessWorker(100), GOTOsender(100), GOTODatabaseWorker(100) {}

public:
    static constexpr size_t maxLoopCount = 32;

    static Router& GetInstance()
    {
        if (instance == nullptr) instance = new Router;
        return *instance;
    }
    static void DeleteInstance()
    {
        if (instance != nullptr) 
        {
            delete instance;
            instance = nullptr;
        }
    }
    ~Router() 
    {}

    bool Initialize(int pkNum);

    bool PushPacket(Packet* pk) {return PacketPool.Push(pk);}
    bool PopPacket(Packet*& pk) {return PacketPool.Pop(pk);}
    
    bool EnqueueElement(PipeType ID, NetElement&& session);
    bool DequeueElementAsChunk(PipeType ID, std::vector<NetElement>& chunk, size_t maxSize = maxLoopCount);

    bool EnqueueDBElement(NetElement&& element);
    bool DequeueDBElementAsChunk(std::vector<NetElement>& chunk, size_t maxSize = maxLoopCount);
};
#endif