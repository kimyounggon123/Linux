#ifndef PROCESSDISPATCHER_H
#define PROCESSDISPATCHER_H

#include <functional>
#include <iostream>

#include "../../Parser/InGame/PlayerProtocol.hpp"
#include "IProcessDispatcher.hpp"

class GeneralProcessDispatcher : public IProcessDispatcher<NetworkTask, Context>
{
public: 
    using GeneralHandler = PacketResult(GeneralProcessDispatcher::*)(NetworkTask&, Context&);

private:

    bool isInitialized;
    static std::array<GeneralHandler, ChangeToUINT(PacketType::LastDummy)> handlers;

    PlayerProtocol player;
    PacketResult Test(NetworkTask& element, Context& context) {return player.Test(element, context);}


public:
    GeneralProcessDispatcher() : IProcessDispatcher(), isInitialized(false) {}
    ~GeneralProcessDispatcher() {}

    // 기초적인 검사
    PacketResult Dispatch(NetworkTask& element, Context& context) override
    {
        const auto type = element.pk->GetTypeUINT();
        if (type >= handlers.size()) return NULL_PACKET_METHOD(element, context);
        return (this->*handlers[type])(element, context);
    }
    bool Initialize() override;
};

#endif