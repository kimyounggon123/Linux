#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>
#include "../../../../Core/Utils/Math/Vectors.hpp"

class Player
{
    uint32_t sessionID;
    uint32_t playerID;
    uint32_t roomID; // 소속 없을 경우 0

    Vector2Float pos;
    std::chrono::steady_clock::time_point lastHeartbeatTime;

    bool isPendingDelete; // 삭제 플래그
public:
    Player(uint32_t sessionID, uint32_t playerID):
        sessionID(sessionID), playerID(playerID), roomID(0),
        pos{0.0f, 0.0f} {}
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    ~Player() {}

    uint32_t GetSessionID() const {return sessionID;}
    uint32_t GetPlayerID() const {return playerID;}
    
    void UpdateHeartbeat() { lastHeartbeatTime = std::chrono::steady_clock::now(); }
    const std::chrono::steady_clock::time_point GetHeartBeatTime() const {return lastHeartbeatTime;}

    const bool IsPendDelete() const {return isPendingDelete;}
    void PendDeleting() {isPendingDelete = true;}

    uint32_t GetRoomID() const {return roomID;}
    void EnterRoom(uint32_t roomID) {this->roomID = roomID;}
    void QuitRoom() {roomID = 0;}
    
    const Vector2Float& GetVector2() const {return pos;}
    void Teleport(const Vector2Float& here) {pos = here;}
    void Move(const Vector2Float& moveUnit) {pos += moveUnit;}


};
#endif