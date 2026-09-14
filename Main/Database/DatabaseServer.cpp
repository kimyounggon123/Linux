#include "DatabaseServer.hpp"

bool DatabaseServer::MakeTaskWorkers()
{   
    if (!dispatcher.Initialize()) return false;
    if (!mariaPool.Initialize(threadPoolCount)) return false;
    if (!redisPool.Initialize(threadPoolCount, 6000)) return false;
    
    std::unique_ptr<DatabaseTaskWorker> worker = nullptr;
    MariaControlList* mariaList = nullptr;
    RedisControl* redis = nullptr;

    for (uint32_t i = 0; i < threadPoolCount * 2; i++)
    {
        mariaList = mariaPool.FindList(i);
        redis = redisPool.Find(i);
        if (mariaList == nullptr || redis == nullptr) 
        {
            return false;
        }
        DatabaseUtils utils = {mariaList, redis};
        worker = std::make_unique<DatabaseTaskWorker>(services, utils, dispatcher,  i);
        if (worker == nullptr) 
        {
            std::cout << "worker " << std::endl;
            return false;
        }
        taskWorkerComponent.InsertProcessWorker(std::move(worker));   
    }

    return true;
}
