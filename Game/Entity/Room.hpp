#ifndef ROOM_H
#define ROOM_H

#include <chrono>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Player.hpp"

class Room 
{
    uint32_t threadID;
    uint32_t ID;

    Player* host;
    std::vector<Player*> players;  

    static constexpr uint64_t TickMs = 50;
    uint64_t nextTickTime;


    bool isPendDelete;

    bool NeedTickUpdate(uint64_t now)
    {
        return now >= nextTickTime;
    }
public:
    Room(uint32_t threadID, uint32_t ID): 
        threadID(threadID), ID(ID),
        nextTickTime(0), host(nullptr), isPendDelete(false) {}
    ~Room()
    {
        players.clear();
    }

    const uint32_t& GetID() const {return ID;}
    const uint32_t& GetThreadID() const {return threadID;}

    bool EnterPlayer(Player* player);
    bool PopPlayer(Player* player);
    Player* GetHost() const {return host;}
    
    bool SetHost(Player* player) 
    {
        if (player == nullptr) return false;
        host = player; return true;
    }

    void PendDelete() {isPendDelete = true;}
    bool IsPendDelete() const {return isPendDelete;}

    // void Update(const GameTask& task);
    void TickUpdate(const uint64_t now);
};

#endif