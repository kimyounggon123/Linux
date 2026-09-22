#include "Room.hpp"

std::chrono::milliseconds Room::SearchInterval = std::chrono::milliseconds(100);

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
    player->QuitWorld();
    return true;
}   

void Room::Update(Clock::duration dt)
{
    UpdateEveryFrame(dt);
    UpdateEachInterval(dt);
}

void Room::UpdateEveryFrame(Clock::duration dt)
{

}
void Room::UpdateEachInterval(Clock::duration dt)
{
    SearchFrameWork(dt);
}
    
void Room::SearchFrameWork(Clock::duration dt)
{
    searchElapsed += dt;
    if (searchElapsed >= SearchInterval) // 탐색 주기가 왔다면
    {
        // Search();
        searchElapsed -= SearchInterval;
    }
}
 