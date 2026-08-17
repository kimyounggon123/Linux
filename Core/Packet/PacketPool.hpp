#ifndef PACKETPOOL_H
#define PACKETPOOL_H

#include "Packet.hpp"
#include "../Utils/Thread/ThreadSafeContainor.hpp"
#include "../Utils/Containor/Containors.hpp"

struct PacketCache
{
    static constexpr uint16_t MAX = 128;
    static constexpr uint16_t Flush = 64;
    std::vector<Packet*> packets;
    PacketCache()
    {
        packets.reserve(MAX);
    }
    ~PacketCache()
    {
        packets.clear();
    }
    bool Push(Packet* p)
    {
        if (p == nullptr || IsFull())
            return false;
        packets.push_back(p);
        return true;
    }
    Packet* Pop()
    {
        if (IsEmpty())
            return nullptr;
        Packet* p = packets.back();
        packets.pop_back();
        return p;
    }

    bool IsEmpty() const { return packets.empty(); }
    bool IsFull() const { return packets.size() == MAX; }
};

class PacketPool
{
    ThreadSafePoolChunkModel<Packet> packetPool;
    static thread_local PacketCache localCache;
    void FillLocalCache();  // cache가 비어있을 경우 다시 채우기
    void EmptyLocalCache(); // cache가 너무 많이 있을 경우 다시 global pool로
public:
    PacketPool(uint32_t maxPoolSize = 9000, uint32_t timeout_ms = INFINITE):
        packetPool(maxPoolSize, timeout_ms)
    {}
    ~PacketPool()
    {
    }

    bool Initialize();
    Packet* Acquire();
    bool Release(Packet* pk);
};


#endif