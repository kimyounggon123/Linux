#ifndef IDISPATCHER_H
#define IDISPATCHER_H


#include "../Packet/Packet.hpp"
#include "../NetworkTask.hpp"

template <typename DispatcherName, typename Utils>
class INetworkTaskDispatcher
{
public:
    using Handler = PacketResult(DispatcherName::*)(NetworkTask&, Utils&);
private:

    static constexpr uint32_t MaxSize = ChangeToUINT(PacketType::LAST_DUMMY);
    std::array<Handler, MaxSize> handlers;
    PacketResult NULL_METHOD(NetworkTask& task, Utils& utils) 
    { 
        task.pk->ClearBuffer();
        return PacketResult::CALL_NULL_METHOD;
    }
protected:

    bool isInitialized;
    bool EmplaceProcess(uint32_t ID, Handler handler)
    {
        if (ID >= MaxSize) return false;
        handlers[ID] = handler;
        return true;
    }

public:  
    INetworkTaskDispatcher(): isInitialized(false)
    {
        handlers.fill(&DispatcherName::NULL_METHOD);
    }
    virtual ~INetworkTaskDispatcher()
    {
        handlers.fill(&DispatcherName::NULL_METHOD);
    }
    virtual bool Initialize() = 0;

    PacketResult Dispatch(uint32_t taskID, NetworkTask& task, Utils& utils)
    {
        if (taskID >= MaxSize || handlers[taskID] == nullptr) return NULL_METHOD(task, utils);
        return (static_cast<DispatcherName*>(this)->*handlers[taskID])(task, utils);
    }
};
#endif