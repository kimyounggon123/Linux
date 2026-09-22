#ifndef GAMEDISPATCHER_H
#define GAMEDISPATCHER_H

#include "../../Core/Server/Packet/Packet.hpp"
#include "../../Core/Server/Dispatcher/INetworkTaskDispatcher.hpp"
#include "../../Core/Server/CoreServices.hpp"
#include "AOI/Object/ObjectManager.hpp"

struct GameUtils
{
    PlayerManager* playerManager;
    RoomManager* roomManager;  
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