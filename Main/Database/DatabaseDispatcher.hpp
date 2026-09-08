#ifndef DATABASEDISPATCHER_H
#define DATABASEDISPATCHER_H

#include "../../Core/Server/Packet/Packet.hpp"
#include "../../Core/Server/Dispatcher/INetworkTaskDispatcher.hpp"
#include "../../Core/Server/CoreServices.hpp"
#include "DBAccountPool.hpp"

struct DatabaseUtils
{
    MariaAccountPool::MariaDBControlList* mariaList;
    RedisControl* redis;

    DatabaseUtils(): mariaList(nullptr), redis(nullptr) {}
    DatabaseUtils(MariaAccountPool::MariaDBControlList* mariaList, RedisControl* redis):
        mariaList(mariaList), redis(redis)  {}

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