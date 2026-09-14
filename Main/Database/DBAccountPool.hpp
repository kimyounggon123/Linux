#ifndef DBACCOUNTPOOL_H
#define DBACCOUNTPOOL_H

#include "Tool/MariaDBControl.hpp"
#include "Tool/RedisControl.hpp"
#include "../../Core/Utils/Containor/Containors.hpp"
#include "../../Core/Utils/utils.h"
enum class DatabaseType
{
    Test,
    Sign,
    LAST_DUMMY
};


class MariaControlList
{
    static constexpr uint32_t MaxSize = ChangeToUINT(DatabaseType::LAST_DUMMY); 
    std::array<MariaDBControl, MaxSize> controlList;    
public:
    MariaControlList(){}
    ~MariaControlList(){}
    bool Connect(DatabaseType type);
    MariaDBControl* Find(DatabaseType type);
};
class MariaAccountPool
{
    PoolUsingKey<MariaControlList> mariaPool;
    bool MakeSign(uint32_t shardCount);
public:
    MariaAccountPool() {}
    ~MariaAccountPool() {}

    bool Initialize(uint32_t shardCount);
    MariaControlList* FindList(uint32_t shardID);
};


class RedisAccountPool
{
    PoolUsingKey<RedisControl> pool; 
public:
    RedisAccountPool(){}
    ~RedisAccountPool(){}


    bool Initialize(uint32_t shardCount, uint32_t startPort = 6000);
    RedisControl* Find(uint32_t shardID);
};

#endif