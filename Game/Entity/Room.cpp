#include "Room.hpp"


void Room::TickUpdate(const uint64_t now)
{
    nextTickTime += now;   
}