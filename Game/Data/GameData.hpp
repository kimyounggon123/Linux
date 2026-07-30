#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "Math/Vectors.hpp"
#include "../Entity/Room.hpp"

struct MoveData
{
    uint32_t playerID;
    Vector2Int start;
    Vector2Int end;
};




#include "../Entity/Player.hpp"
#include <variant>
enum class GameTaskType
{
    Move,
    LAST_DUMMY
};
class Room;  // 전방 선언
struct GameTask
{
    GameTaskType type;
    Player* player;
    Room* room;
    union 
    {
        MoveData move;
    };
    GameTask(GameTaskType type, Player* player, Room* room): type(type), player(player), room(room)
    {}
};
#endif