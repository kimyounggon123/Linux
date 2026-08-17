#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "Math/Vectors.hpp"
#include <variant>

struct MoveData
{
    uint32_t playerID;
    //int x; int y;
    Vector2Int start;
    Vector2Int end;

    MoveData(): playerID(0) // x(0), y(0) 
    {}
    ~MoveData() {}
};


#endif