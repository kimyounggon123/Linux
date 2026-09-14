#ifndef DATABASEDISPATCHER_H
#define DATABASEDISPATCHER_H

#include "../../Core/Server/Packet/Packet.hpp"
#include "../../Core/Server/Dispatcher/INetworkTaskDispatcher.hpp"
#include "../../Core/Server/CoreServices.hpp"
#include "DBAccountPool.hpp"

struct DatabaseUtils
{
    MariaControlList* mariaList;
    RedisControl* redis;

    DatabaseUtils(): mariaList(nullptr), redis(nullptr) {}
    DatabaseUtils(MariaControlList* mariaList, RedisControl* redis):
        mariaList(mariaList), redis(redis)  {}
    DatabaseUtils(const DatabaseUtils& other):
        mariaList(other.mariaList), redis(other.redis)
    {}
    void Clear()
    {
        mariaList = nullptr;
        redis = nullptr;
    }
};


class DatabaseDispatcher : public INetworkTaskDispatcher<DatabaseDispatcher, DatabaseUtils>
{
    PacketResult LogIn(NetworkTask& task, DatabaseUtils& utils);
public:  
    DatabaseDispatcher(): INetworkTaskDispatcher() {}
    ~DatabaseDispatcher(){}
    bool Initialize() override;
};

#endif