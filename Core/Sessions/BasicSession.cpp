#include "BasicSession.hpp"


/*
Room::Room(int roomID, int maxClientsNum) :
    isAlive(true),
    roomID(roomID), maxClientsNum(maxClientsNum), currClientNum(0)
{
    room.reserve(maxClientsNum);
}
Room::~Room()
{

}

bool Room::AddClient(LinuxSession* client)
{
    if (isAlive.load() == false || client == nullptr || currClientNum == maxClientsNum) return false;

    auto pair = room.emplace(client->ID, client);

    if (!pair.second) return false;

    currClientNum++;
    return true;
}
LinuxSession* Room::FindClient(int id)
{
    LinuxSession* found = nullptr;
    if (isAlive == false) return found;
    auto it = room.find(id);

    if (it != room.end())
    {
        found = it->second;
    }
    return found;
}

bool Room::DeleteClient(int id)
{
    if (isAlive.load() == false) return false;
    auto it = room.find(id);
    if (it == room.end()) return false;
    
    //delete it->second;
    //it->second = nullptr;

    room.erase(id);
    currClientNum--;
    return true;
}
bool Room::DeleteClient(const LinuxSession& client)
{
    return DeleteClient(client.ID);
}


void Room::Destroy()
{
    isAlive.store(false);
    for (auto it = room.begin(); it != room.end();)
    {
        //delete it->second;
        //it->second = nullptr;

        it = room.erase(it);
    }
}

*/
