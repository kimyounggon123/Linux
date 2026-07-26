#ifndef PLAYERMANAER_H
#define PLAYERMANAER_H

#include <vector>
#include "Player.hpp"

class Room 
{
    uint32_t ownerThreadID;
    uint32_t ID;
    std::vector<Player*> players;  
public:
    Room(uint32_t ID, uint32_t ownerThreadID): ID(ID), ownerThreadID(ownerThreadID)
    {}
    ~Room() = default;

    const uint32_t& GetID() const {return ID;}
    
    const uint32_t& GetOwnerThreadID() const {return ownerThreadID;}
    void SetOwnerThreadID(uint32_t id) {ownerThreadID = id;}

    bool Enter(Player* player) 
    {
        if (player == nullptr) return false;
        players.push_back(player);
        return true;
    }
};

#endif