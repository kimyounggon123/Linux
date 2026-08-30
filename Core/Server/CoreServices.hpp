#ifndef CORESERVICES_H
#define CORESERVICES_H

#include "Pipe/PipePool.hpp"
#include "Packet/PacketPool.hpp"
#include "Manager/SessionManager.hpp"


using NetWorkPipePool = PipePool<NetworkTask>;
struct CoreServices
{
    NetWorkPipePool* processPipePool;
    NetWorkPipePool* sendPipePool;
    PacketPool* pkPool;
    SessionManager* sessionManager;

    CoreServices(): 
        processPipePool(nullptr), sendPipePool(nullptr), pkPool(nullptr), sessionManager(nullptr) {}

    CoreServices(NetWorkPipePool* processPipePool, NetWorkPipePool* sendPipePool,
        PacketPool* pkPool, SessionManager* sessionManager):
        processPipePool(processPipePool), sendPipePool(sendPipePool),
        pkPool(pkPool), sessionManager(sessionManager) 
    {}

    CoreServices(const CoreServices& other):
        processPipePool(other.processPipePool), sendPipePool(other.sendPipePool),
        pkPool(other.pkPool), sessionManager(other.sessionManager)  
    {}
};
#endif