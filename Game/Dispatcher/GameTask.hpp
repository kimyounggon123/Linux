#ifndef GAMETASK_H
#define GAMETASK_H

#include "../Data/GameData.hpp"
#include "../../Core/Utils/utils.h"
enum class GameTaskType
{
    Test,
    Move,
    LAST_DUMMY
};
enum class GameTaskResult : uint32_t
{
    Fail        = 0,
    Success     = 1 << 0,
    Broadcast   = 1 << 1,
    SaveDB      = 1 << 2,
};
inline GameTaskResult operator|(GameTaskResult a, GameTaskResult b)
{
    return static_cast<GameTaskResult>(
        static_cast<uint32_t>(a) |
        static_cast<uint32_t>(b)
    );
}
inline bool HasFlag(GameTaskResult value, GameTaskResult flag)
{
    return (static_cast<uint32_t>(value) &
            static_cast<uint32_t>(flag)) != 0;
}


struct GameTask
{
    bool ignoreThis;
    GameTaskType type;
    GameTaskResult result;

    uint32_t playerID;
    uint32_t roomID;
    uint32_t sessionID;
    
    union 
    {
        MoveData move;
    };

    GameTask(GameTaskType type, uint32_t playerID, uint32_t roomID, uint32_t sessionID):
        ignoreThis(false),
        type(type), result(GameTaskResult::Fail), 
        playerID(playerID), roomID(roomID), sessionID(sessionID)
    {}

    ~GameTask(){}
};


/*
    Game Task의 종류
    1. Player 단독 설정
    2. Room 단독 설정(이건 worker loop 마다 tick work로 하는 것이 적절해보임)
    3. Player, Room 동시 설정

    문제점:
    기본적으로 3번 경우를 worker의 basic logic으로 설정했으나, 
    1번 경우를 상정해야 한다면 어떻게 해야 하는가?

    예시: 
    room->Update(task)가 기본 task이지만
    player->Update(task)도 필요할 수 있음.
    
    1. player->Update()를 기본 task로 지정.
    2. 두 개의 분기로 나눔
*/

#endif