#ifndef SIGNDISPATCHER_H
#define SIGNDISPATCHER_H


#include "../../Core/Server/Packet/Packet.hpp"
#include "../../Core/Server/Dispatcher/INetworkTaskDispatcher.hpp"
#include "../../Core/Server/CoreServices.hpp"

struct SignUtilEx
{
    PacketPool* pkPool;
    SessionManager* sessionManager;
    PipePool<Packet*>* toSendDB;
    SignUtilEx(PacketPool* pkPool, SessionManager* sessionManager, PipePool<Packet*>* toSendDB):
        pkPool(pkPool), sessionManager(sessionManager), toSendDB(toSendDB) {}
};


class SignDispatcher : public INetworkTaskDispatcher<SignDispatcher, SignUtilEx>
{
    PacketResult LogIn(NetworkTask& task, SignUtilEx& utils);
public:  
    SignDispatcher(): INetworkTaskDispatcher() {}
    ~SignDispatcher(){}
    bool Initialize() override;
};

#endif