#ifndef IPACKETFACTORY_H
#define IPACKETFACTORY_H

#include "Packet.hpp"
#include "PacketPool.hpp"
#include "../Utils/Log.hpp"
template <typename FactoryName, typename T, size_t FactorySize>
class IPacketFactory
{
public:
    using FactoryProcess = bool(FactoryName::*)(Packet&, T&);
    
protected:
    std::array<FactoryProcess, FactorySize> processes;
    PacketPool& pkPool;
    bool isInitialized;

    bool NULL_METHOD(Packet& pk, T& task)  {return false;}
public:
    IPacketFactory(PacketPool& pkPool): pkPool(pkPool), isInitialized(false)
    {}
    virtual ~IPacketFactory() {}  

    Packet* MakePacket(T& task)
    {
        Packet* pk = pkPool.Acquire();
        if (pk == nullptr) return nullptr;
        pk->ClearBuffer();
        if (!(static_cast<FactoryName*>(this)->*processes[ChangeToUINT(task.type)])(*pk, task)) return nullptr;
        return pk;
    }

    virtual bool Initialize() = 0;
};

#endif