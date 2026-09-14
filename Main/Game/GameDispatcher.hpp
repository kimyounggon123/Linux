#ifndef GAMEDISPATCHER_H
#define GAMEDISPATCHER_H

#include "../../Core/Server/Packet/Packet.hpp"
#include "../../Core/Server/Dispatcher/INetworkTaskDispatcher.hpp"
#include "../../Core/Server/CoreServices.hpp"


struct GameUtils
{

};

class GameDispatcher : public INetworkTaskDispatcher<GameDispatcher, GameUtils>
{
    PacketResult LogIn(NetworkTask& task, GameUtils& utils);
public:  
    GameDispatcher(): INetworkTaskDispatcher() {}
    ~GameDispatcher(){}
    bool Initialize() override;
};

#endif