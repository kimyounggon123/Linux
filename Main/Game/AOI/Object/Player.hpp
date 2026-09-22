#ifndef PLAYER_H
#define PLAYER_H

#include <chrono>
#include "../Math/Vectors.hpp"

class Player
{
    uint32_t sessionID;
    uint32_t playerID;

    uint32_t worldID; // 소속 있어야 함.
    uint32_t roomID; // 소속 없을 경우 0

    Vector2Float pos;
    std::chrono::steady_clock::time_point lastHeartbeatTime;

    bool isPendingDelete; // 삭제 플래그


public:
    Player(uint32_t sessionID, uint32_t playerID, uint32_t worldID):
        sessionID(sessionID), playerID(playerID), worldID(worldID), roomID(0),
        pos{0.0f, 0.0f},
        isPendingDelete(false)
    {}
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    ~Player() {}

    uint32_t GetSessionID() const {return sessionID;}
    uint32_t GetPlayerID() const {return playerID;}
    
    void UpdateHeartbeat() { lastHeartbeatTime = std::chrono::steady_clock::now(); }
    const std::chrono::steady_clock::time_point GetHeartBeatTime() const {return lastHeartbeatTime;}

    const bool IsPendDelete() const {return isPendingDelete;}
    void PendDeleting() {isPendingDelete = true;}

    uint32_t GetWorldID() const {return worldID;}
    void EnterWorld(uint32_t worldID) {this->worldID = worldID;}
    void QuitWorld() {worldID = 0;}

    uint32_t GetRoomID() const {return roomID;}
    void EnterRoom(uint32_t roomID) {this->roomID = roomID;}
    void QuitRoom() {roomID = 0;}
    
    const Vector2Float& GetVector2() const {return pos;}

    void Teleport(const Vector2Float& here) {pos = here;}
    void Move(const Vector2Float& moveUnit) {pos += moveUnit;}
};
#endif