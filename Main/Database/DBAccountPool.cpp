#include "DBAccountPool.hpp"
bool MariaControlList::Connect(DatabaseType type)
{
    uint32_t typeInt = ChangeToUINT(type);
    if (typeInt >= MaxSize) return false;

    bool result = true;
    MariaDBControl* controller = &controlList[typeInt];
    switch (type)
    {
        case DatabaseType::Sign:
            result = controller->Connect("localhost", "TestUser", "test", "TestDatabase");
            break;
    }
    return result;
}
MariaDBControl* MariaControlList::Find(DatabaseType type)
{
    uint32_t typeInt = ChangeToUINT(type);
    if (typeInt >= MaxSize) return nullptr;
    return &controlList[typeInt];
}



bool MariaAccountPool::MakeSign(uint32_t shardCount)
{
    for (uint32_t i = 0; i < shardCount; i++)
    {
        MariaControlList* controlList = mariaPool.GetElement(i);
        if (controlList == nullptr) 
        {
            std::cout << "Empty Maria List" << std::endl;
            return false;
        }
        if (!controlList->Connect(DatabaseType::Sign)) return false;
    }
    return true;
}

bool MariaAccountPool::Initialize(uint32_t shardCount)
{
    bool result = true;
    for (uint32_t i = 0; i < shardCount; i++)
    {
        std::unique_ptr<MariaControlList> controls = std::make_unique<MariaControlList>();
        if (controls == nullptr) 
        {
            return false;
        }
        if (!mariaPool.AddElement(std::move(controls)))
        {
            return false;
        } 
    }
    result = result && MakeSign(shardCount);
    return result;
}

MariaControlList* MariaAccountPool::FindList(uint32_t shardID)
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
        if (!account->Connect("127.0.0.1")) 
        {
            std::cout << "Redis Connect Error" << std::endl;
            return false;
        }
        if (!pool.AddElement(std::move(account))) return false;
    } 
    return true;
}

RedisControl* RedisAccountPool::Find(uint32_t shardID)
{
    return pool.GetElement(shardID);
}