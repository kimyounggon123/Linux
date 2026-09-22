#ifndef ROOM_H
#define ROOM_H

#include <vector>
#include <array>
#include "Player.hpp"

using Clock = std::chrono::steady_clock;

class Room
{  
    uint32_t ID;
    std::vector<Player*> players;

    // 매 틱마다 처리
    void UpdateEveryFrame(Clock::duration dt);

    // 개별 주기 처리
    void UpdateEachInterval(Clock::duration dt);

    Clock::duration searchElapsed;
    static std::chrono::milliseconds SearchInterval; // 100 ms
    void SearchFrameWork(Clock::duration dt);

public:
    Room(uint32_t ID): ID(ID), searchElapsed{0} {}
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

    void Update(Clock::duration dt);
};



#endif