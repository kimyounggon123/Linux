#ifndef PAKCETFACTORY_H
#define PAKCETFACTORY_H

#include "../../Core/Contexts/Contexts.hpp"
#include "../../Core/Contexts/NetworkTask.hpp"
#include "../Data/GameData.hpp"
class PacketFactory
{
    PacketPool& pkPool;
    SessionManager& sessionManager;
    Router& router;

    BasicSession* FindSession(uint32_t id);
public:
    PacketFactory(Context& context): 
        pkPool(context.packetPool), sessionManager(context.sessionManager), 
        router(context.router)
    {}
    ~PacketFactory(){}

    bool Send(const GameTask& task);
};
#endif