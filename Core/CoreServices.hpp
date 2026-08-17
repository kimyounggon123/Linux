#ifndef CORESERVICES_H
#define CORESERVICES_H

#include "Pipe/PipePool.hpp"
#include "Packet/PacketPool.hpp"
#include "Manager/SessionManager.hpp"

struct CoreServices
{
    NetWorkPipePool& processPipePool;
    NetWorkPipePool& sendPipePool;
    PacketPool& pkPool;
    SessionManager& sessionManager;
    
    CoreServices(NetWorkPipePool& processPipePool, NetWorkPipePool& sendPipePool,
        PacketPool& pkPool, SessionManager& sessionManager):
        processPipePool(processPipePool), sendPipePool(sendPipePool),
        pkPool(pkPool), sessionManager(sessionManager)
    {}
};


#include "../Database/ServerLinker/DBPipePool.hpp"
#include "../Game/ServerLinker/AOIPipe.hpp"
struct ComponentConnections
{
    DBPipePool* toDB;
    AOIPipePool* toAOI;

    ComponentConnections() :
        toDB(nullptr),  toAOI(nullptr) {}

    ComponentConnections(DBPipePool* toDB, AOIPipePool* toAOI):
        toDB(toDB),  toAOI(toAOI) {}

};
#endif