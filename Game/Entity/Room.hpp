#ifndef ROOM_H
#define ROOM_H

#include <chrono>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Player.hpp"
#include "../Data/GameData.hpp"


class Room 
{
    uint32_t ID;
    std::vector<Player*> players;  
    static constexpr uint64_t TickMs = 50;
    uint64_t nextTickTime;
public:
    Room(uint32_t ID, uint32_t ownerThreadID): ID(ID), nextTickTime(0) {}
    ~Room() = default;

    const uint32_t& GetID() const {return ID;}
    bool EnterPlayer(Player* player) 
    {
        if (player == nullptr) return false;
        players.push_back(player);
        return true;
    }
    bool PopPlayer(Player* player)
    {
        if (player == nullptr) return false;
        return true;
    }
    bool NeedTick(uint64_t now)
    {
        return now >= nextTickTime;
    }

    void Update(const GameTask& task);
    void TickUpdate(const uint64_t now);
};

#include "../../Core/Utils/Containor/Containors.hpp"
class RoomManager
{
    ElementRegistry<uint32_t, Room> rooms;   
public:
    RoomManager(){}
    ~RoomManager(){}

    bool AddRoom();
};
#endif