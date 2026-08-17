#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include "Room.hpp"
#include "Player.hpp"

#include "../../Core/Utils/Containor/Containors.hpp"
#include "../../Core/Utils/Thread/ThreadSafeContainor.hpp"
#include "../../Core/Manager/IManager.hpp"



// thread safe
class PlayerManager : public IManager
{
    std::atomic<uint32_t> nextID;
    ThreadElementRegistry<uint32_t, Player> players; 
    ThreadSafeContainor<Player*> deletedList;

    bool Delete(const uint32_t id) { return players.Delete(id); }
    void DeletePendList();
public:
    PlayerManager(): IManager(), nextID(0) {}
    ~PlayerManager(){}

    Player* Create();
    Player* Find(const uint32_t id) { return players.Find(id); }

    bool PendDelete(Player* room);
    bool PendDelete(const uint32_t id);

    std::vector<Player*>& GetList() { return players.GetObjects(); }
    void Process() override
    {
        DeletePendList();
    }
};


// thread local
class RoomManager : public IManager
{
    uint32_t threadID;
    std::atomic<uint32_t> nextID;
    ElementRegistry<uint32_t, Room> rooms; 
    std::vector<Room*> deletedList;
    bool Delete(const uint32_t id) { return rooms.Delete(id); }
    void DeletePendList();
public:
    RoomManager(uint32_t threadID): IManager(), threadID(threadID), nextID(0) {}
    ~RoomManager(){}

    Room* Create();
    Room* Find(const uint32_t id) { return rooms.Find(id); }

    bool PendDelete(Room* room);
    bool PendDelete(const uint32_t id);

    const uint32_t& GetThreadID() const {return threadID;}
    std::vector<Room*>& GetList() { return rooms.GetObjects(); }
    void Process() override
    {
        DeletePendList();
    }
};

//using RoomManagerPool = PoolUsingKey<RoomManager>;
#endif