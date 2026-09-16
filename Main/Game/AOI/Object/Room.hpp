#ifndef ROOM_H
#define ROOM_H

#include <vector>
#include <array>

#include "Player.hpp"

class Room
{
    uint32_t ID;
    std::vector<Player*> players;

public:
    Room(uint32_t ID): ID(ID) {}
    ~Room()
    {
        for (auto* player : players) player->QuitRoom();
        players.clear();
    }

    uint32_t GetID() const {return ID;}
    bool EnterPlayer(Player* player);
    bool QuitPlayer(Player* player);

    size_t GetPlayerCount() const {return players.size();}
    bool IsEmpty() const {return players.size() == 0;}
};



#endif