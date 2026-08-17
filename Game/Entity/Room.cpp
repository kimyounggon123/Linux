#include "Room.hpp"


bool Room::EnterPlayer(Player* player) 
{
    if (player == nullptr) return false;
    player->SetRoomID(ID);
    players.push_back(player);
    return true;
}
bool Room::PopPlayer(Player* player)
{
    if (player == nullptr) return false;
    for (auto it = players.begin(); it != players.end(); it++)
    {
        if (*it == player)
        {
            players.erase(it);
            return true;
        }
    }
    return false;
}

void Room::TickUpdate(const uint64_t now)
{
    nextTickTime += now;   
}

