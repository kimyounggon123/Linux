#include "EntityManager.hpp"



void PlayerManager::DeletePendList()
{
    std::vector<Player*> temp; 
    deletedList.Swap(temp); 
    for (auto* room : temp)
    {
        Delete(room->GetSessionID());
    }   
}
Player* PlayerManager::Create()
{
    uint32_t id = nextID.fetch_add(1, std::memory_order_relaxed);
    std::unique_ptr<Player> ptr = std::make_unique<Player>(id);
    if (ptr == nullptr) return nullptr;
    Player* player = ptr.get();
    players.AddElement(id, std::move(ptr));
    return player;
}
bool PlayerManager::PendDelete(Player* player)
{
    if (player == nullptr) return false;
    player->PendDelete();
    deletedList.Push(player);
    return true;
}
bool PlayerManager::PendDelete(const uint32_t id)
{   
    return PendDelete(Find(id));
}


void RoomManager::DeletePendList()
{
    std::vector<Room*> temp;  
    deletedList.swap(temp);
    for (auto* room : temp)
    {
        Delete(room->GetID());
    }   
}
Room* RoomManager::Create()
{
    uint32_t id = nextID.fetch_add(1, std::memory_order_relaxed);
    std::unique_ptr<Room> newRoom = std::make_unique<Room>(threadID, id);
    if (newRoom == nullptr) return nullptr;
    Room* room = newRoom.get();
    rooms.AddElement(id, std::move(newRoom));
    return room;
}
bool RoomManager::PendDelete(Room* room)
{
    if (room == nullptr) return false;
    room->PendDelete();
    deletedList.push_back(room);
    return true;
}
bool RoomManager::PendDelete(const uint32_t id)
{   
    return PendDelete(Find(id));
}

