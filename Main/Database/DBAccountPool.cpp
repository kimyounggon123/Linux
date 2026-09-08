#include "DBAccountPool.hpp"
bool MariaAccountPool::MakeSign(uint32_t shardCount)
{
    
    for (uint32_t i = 0; i < shardCount; i++)
    {
        MariaDBControlList* controlList = mariaPool.GetElement(i);
        if (controlList == nullptr) return false;
        
        MariaDBControl& account = (*controlList)[ChangeToUINT(DatabaseType::Sign)];
        if (!account.Connect("localhost", "TestUser", "test", "TestDatabase")) return false;
    }
    return true;
}

bool MariaAccountPool::Initialize(uint32_t shardCount)
{
    bool result = true;
    for (uint32_t i = 0; i < shardCount; i++)
    {
        std::unique_ptr<MariaDBControlList> controls = std::make_unique<MariaDBControlList>();
        if (controls == nullptr) return false;
        if (mariaPool.AddElement(std::move(controls))) return false;
    }
    result = result && MakeSign(shardCount);
    return result;
}

MariaAccountPool::MariaDBControlList* MariaAccountPool::FindList(uint32_t shardID)
{
    return mariaPool.GetElement(shardID);  
}


bool RedisAccountPool::Initialize(uint32_t shardCount, uint32_t startPort)
{
    std::unique_ptr<RedisControl> account = nullptr;
    for (uint32_t i = 0; i < shardCount; i++)
    {
        account = std::make_unique<RedisControl>();
        if (account == nullptr) return false;
        if (!account->Connect("tcp//127.0.0.1", startPort + i)) return false;
        if (!pool.AddElement(std::move(account))) return false;
    } 
    return true;
}

RedisControl* RedisAccountPool::Find(uint32_t shardID)
{
    return pool.GetElement(shardID);
}