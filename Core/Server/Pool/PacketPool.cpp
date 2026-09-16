#include "PacketPool.hpp"
// thread_local PacketCache PacketPool::localCache;

// bool PacketPool::Initialize()
// {
//     for (uint32_t i = 0; i < packetPool.GetCapacity(); i++)
//     {
//         std::unique_ptr<Packet> pk = std::make_unique<Packet>();
//         if (pk == nullptr) return false;
//         packetPool.AddElement(std::move(pk));
//     }
//     return true;
// }

// void PacketPool::FillLocalCache()
// {
//     packetPool.PopChunk(localCache.packets, localCache.MAX);
// }  
// void PacketPool::EmptyLocalCache()
// {
//     packetPool.PushChunk(localCache.packets, localCache.Flush);
// }

// Packet* PacketPool::Acquire()
// {
//     if (localCache.IsEmpty()) FillLocalCache();
//     return localCache.Pop();
// }

// bool PacketPool::Release(Packet* pk)
// {
//     if (localCache.IsFull()) EmptyLocalCache();
//     return localCache.Push(pk);
// }