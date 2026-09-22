#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "Player.hpp"
#include "Room.hpp"
#include "../../../../Core/Server/Manager/IManager.hpp"


// These classes are not global.
class PlayerManager 
{
    static uint32_t timeOut;
    uint32_t shardID;
    uint32_t nextID;

    ElementRegistry<uint32_t, Player> players;
    std::vector<Player*> quitedPlayerList;

    bool DeletePlayer(uint32_t playerID);
    bool DeletePlayer(Player* player); 

public:
    PlayerManager(uint32_t shardID): shardID(shardID), nextID(0) {}
    ~PlayerManager() {}

    Player* CreatePlayer(uint32_t sessionID);
    Player* FindPlayer(uint32_t playerID);

    bool PendDelete(uint32_t playerID);
    bool PendDelete(Player* player);

    void CheckPlayersLifeTime(); // 1
    std::vector<Player*>& GetQuitList() {return quitedPlayerList;} // 2
    void DeleteQuitedPlayer(); // 가장 마지막에 호출
};


class RoomManager
{
    uint32_t shardID;
    uint32_t nextID;
    ElementRegistry<uint32_t, Room> rooms;
public:
    RoomManager(uint32_t shardID): shardID(shardID), nextID(0) {}
    ~RoomManager() {}

    Room* CreateRoom();
    Room* FindRoom(uint32_t roomID);

    void DeleteQuitedPlayerInRoom(std::vector<Player*>& quitedList); // 3
    void DeleteEmptyRoom(); // 4

    void Update(Clock::duration dt);
};
#endif