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


class MariaAccountPool
{
    static constexpr uint32_t MaxSize = ChangeToUINT(DatabaseType::LAST_DUMMY);   

public:
    using MariaDBControlList = std::array<MariaDBControl, MaxSize>;
    using MariaPool =  PoolUsingKey<MariaDBControlList>;
    //std::array<PoolUsingKey<MariaDBControl>, MaxSize>; 

private:
    MariaPool mariaPool;
    bool MakeSign(uint32_t shardCount);
public:
    MariaAccountPool() {}
    ~MariaAccountPool() {}

    bool Initialize(uint32_t shardCount);
    MariaDBControlList* FindList(uint32_t shardID);
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