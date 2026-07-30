#ifndef PLAYER_H
#define PLAYER_H

#include "Data/GameData.hpp"
class Player
{
    uint32_t sessionID;
    uint32_t roomID;

    Vector2Float pos;
public:
    Player(uint32_t sessionID, uint32_t roomID):
        sessionID(sessionID), roomID(roomID), pos{0.0f, 0.0f} 
    {}

    const uint32_t& GetSessionID() const {return sessionID;}
    const uint32_t& GetRoomID() const {return roomID;}
    void SetRoomID(const uint32_t& id) {roomID = id;}
    
    Vector2Float GetPos() const {return pos;}
    void SetPos(const Vector2Float& pos) {this->pos = pos;}

    void Update(const GameTask& task);
};


class PlayerManager
{

};
#endif