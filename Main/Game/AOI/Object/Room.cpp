#include "Room.hpp"


bool Room::EnterPlayer(Player* player)
{
    if (player == nullptr) return false;
    player->EnterRoom(ID);
    players.push_back(player);
    return true;
}
bool Room::QuitPlayer(Player* player)
{
    if (player == nullptr) return false;

    for (auto it = players.begin(); it != players.end(); it++)
    {
        Player* member = *it;
        if (player->GetPlayerID() == member->GetPlayerID())
        {
            players.erase(it);
            break;
        }
    }
    player->QuitRoom();
    return true;
}   