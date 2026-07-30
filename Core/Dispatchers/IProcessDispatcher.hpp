#ifndef IPROCESSDISPATCHER_H
#define IPROCESSDISPATCHER_H

#include "../Contexts/NetworkTask.hpp"
#include "../Contexts/Contexts.hpp"

template <typename E, typename C>
class IProcessDispatcher
{
protected:
    PacketResult NULL_PACKET_METHOD(E& param, C& context)
    {
        return PacketResult::CALL_NULL_METHOD;
    }
public:
    IProcessDispatcher() {}
    virtual ~IProcessDispatcher() {}

    virtual PacketResult Dispatch(E& element, C& context) = 0;
    virtual bool Initialize() = 0;
};

#endif