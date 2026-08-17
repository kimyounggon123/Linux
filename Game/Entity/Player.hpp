#ifndef PLAYER_H
#define PLAYER_H

#include "../Data/GameData.hpp"
class Player
{
    uint32_t id;
    uint32_t sessionID;
    uint32_t roomID;
    Vector2Float pos;
    bool isPendDelete;

public:
    Player(uint32_t id):
        id(id), sessionID(0), roomID(roomID), pos{0.0f, 0.0f} 
    {}

    const uint32_t& GetSessionID() const {return sessionID;}
    const uint32_t& GetRoomID() const {return roomID;}
    void SetRoomID(const uint32_t& id) {roomID = id;}
    
    Vector2Float GetPos() const {return pos;}
    void SetPos(const Vector2Float& pos) {this->pos = pos;}

    void PendDelete() {isPendDelete = true;}
    bool IsPendDelete() const {return isPendDelete;}

    //void Update(const GameTask& task);
};

#endif