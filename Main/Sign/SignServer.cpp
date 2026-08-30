#include "SignServer.hpp"


bool SignServer::MakeTaskWorkers()
{
    dispatcher.Initialize();
    std::unique_ptr<SignTaskWorker> worker = nullptr;
    for (uint32_t i = 0; i < threadPoolCount * 2; i++)
    {
        worker = std::make_unique<SignTaskWorker>(services, utils, dispatcher,  i);
        if (worker == nullptr) return false;
        taskWorkerComponent.InsertProcessWorker(std::move(worker));   
    }
    return true;
}