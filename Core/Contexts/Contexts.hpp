#ifndef CONTEXTS_H
#define CONTEXTS_H

#include "../Pipe/Router.hpp"
#include "../Sessions/SessionManager.hpp"
#include "../Packet/PacketPool.hpp"

struct Context
{
    PacketPool& packetPool;
    Router& router;
    SessionManager& sessionManager;
    Context(PacketPool& packetPool, Router& router, SessionManager& sessionManager):
        packetPool(packetPool), router(router), sessionManager(sessionManager) 
    {}
};

#include "../Database/RedisController.hpp"
struct DBContext
{
    Context& general;
    RedisController& redis; 

    DBContext(Context& general, RedisController& redis):
        general(general), redis(redis)
    {}
};

#endif