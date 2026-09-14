#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Player.hpp"
#include "Room.hpp"
#include "../../../../Core/Server/Manager/IManager.hpp"



// These classes are not global.
class PlayerManager 
{

    uint32_t shardID;
    ElementRegistry<uint32_t, Player> players;
    void CheckHeartbeats();
public:
    PlayerManager(uint32_t shardID): shardID(shardID) {}
    ~PlayerManager() {}


    bool AddPlayer(std::unique_ptr<Player> player);
    Player* FindPlayer(uint32_t playerID);

    bool DeletePlayer(uint32_t playerID);
    bool DeletePlayer(Player* player);
    
};

#endif