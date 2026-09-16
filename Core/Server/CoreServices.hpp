#ifndef CORESERVICES_H
#define CORESERVICES_H

#include "Pool/PipePool.hpp"
#include "Pool/BroadcastTaskPool.hpp"
#include "Pool/PacketPool.hpp"
#include "Manager/SessionManager.hpp"

using NetWorkPipePool = PipePool<NetworkTask>;
using BroadcastTaskPipePool = PipePool<BroadcastTask*>;

struct CoreServices
{
    NetWorkPipePool* processPipePool;
    NetWorkPipePool* sendPipePool;
    BroadcastTaskPipePool* broadcastPipePool;

    PacketPool* pkPool;
    BroadcastTaskPool* broadPool;
    SessionManager* sessionManager;

    CoreServices(): 
        processPipePool(nullptr), sendPipePool(nullptr), broadcastPipePool(nullptr),
        pkPool(nullptr), broadPool(nullptr),
        sessionManager(nullptr) {}

    CoreServices(NetWorkPipePool* processPipePool, NetWorkPipePool* sendPipePool, BroadcastTaskPipePool* broadcastPipePool,
        PacketPool* pkPool, BroadcastTaskPool* broadPool, SessionManager* sessionManager):
        processPipePool(processPipePool), sendPipePool(sendPipePool), broadcastPipePool(broadcastPipePool),
        pkPool(pkPool), broadPool(broadPool),
        sessionManager(sessionManager) 
    {}

    CoreServices(const CoreServices& other):
        processPipePool(other.processPipePool), sendPipePool(other.sendPipePool), broadcastPipePool(other.broadcastPipePool),
        pkPool(other.pkPool), broadPool(other.broadPool),
        sessionManager(other.sessionManager)  
    {}
};
#endif