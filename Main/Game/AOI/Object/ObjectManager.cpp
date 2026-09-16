#include "ObjectManager.hpp"

uint32_t PlayerManager::timeOut = 30;
bool PlayerManager::DeletePlayer(uint32_t playerID)
{
    return players.Delete(playerID);
}
bool PlayerManager::DeletePlayer(Player* player)
{
    return DeletePlayer(player->GetPlayerID());
}
Player* PlayerManager::CreatePlayer(uint32_t sessionID)
{
    uint32_t playerID = nextID++;
    std::unique_ptr<Player> player = std::make_unique<Player>(sessionID, playerID); 
    if (player == nullptr) return nullptr;

    Player* retval = player.get();
    players.AddElement(playerID, std::move(player));

    return retval;
}
Player* PlayerManager::FindPlayer(uint32_t playerID)
{
    return players.Find(playerID);
}


bool PlayerManager::PendDelete(Player* player)
{
    if (player == nullptr) return false;
    player->PendDeleting();
    quitedPlayerList.push_back(player);
    return true;
}
bool PlayerManager::PendDelete(uint32_t playerID)
{
    return PendDelete(FindPlayer(playerID));
}

void PlayerManager::CheckPlayersLifeTime()
{
    auto now = std::chrono::steady_clock::now();
    const std::chrono::seconds timeoutDuration(timeOut); // n초 동안 하트비트 없으면 끊음

    std::vector<Player*>& tempList = players.GetObjects();
    for (auto* player : tempList)
    {
        // 무시 조건
        if (player == nullptr || player->IsPendDelete()) 
        {
            continue;
        }

        // [핵심] 현재 시간과 마지막 하트비트 시간 비교
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - player->GetHeartBeatTime());
        if (elapsed > timeoutDuration)
        {
            PendDelete(player);
        }
    }
}
void PlayerManager::DeleteQuitedPlayer()
{
    for (auto* player : quitedPlayerList)
    {
        DeletePlayer(player);
    }
}


Room* RoomManager::CreateRoom()
{
    uint32_t roomID = nextID++;
    std::unique_ptr<Room> room = std::make_unique<Room>(roomID);
    if (room == nullptr) return nullptr;
    Room* retval = room.get();
    rooms.AddElement(roomID, std::move(room));
    return retval;
}
Room* RoomManager::FindRoom(uint32_t roomID)
{
    return rooms.Find(roomID);
}

void RoomManager::DeleteQuitedPlayerInRoom(std::vector<Player*>& quitedList)
{
    Room* room = nullptr;
    for (auto* player: quitedList)
    {
        room = FindRoom(player->GetRoomID());
        if (room == nullptr) continue;
        room->QuitPlayer(player);
    }
}
void RoomManager::DeleteEmptyRoom()
{
    std::vector<Room*>& tempList = rooms.GetObjects();
    for (auto* room : tempList)
    {
        if (!room->IsEmpty()) continue;
        rooms.Delete(room->GetID());
    }
}

