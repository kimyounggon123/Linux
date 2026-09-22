#include "GameServer.hpp"


bool GameServer::MakeTaskWorkers() 
{
    if (!dispatcher.Initialize()) return false;


    std::unique_ptr<GameTaskWorker> worker = nullptr;
    for (uint32_t i = 0; i < threadPoolCount * 2; i++)
    {
        GameUtils utils = {};
        worker = std::make_unique<GameTaskWorker>(services, utils, dispatcher,  i);
        if (worker == nullptr) return false;
        taskWorkerComponent.InsertProcessWorker(std::move(worker));   
    }    

    return true;
}


void GameServer::Start()
{
    UDPServer::Start();
    aoiComponent.Start();
}
void GameServer::Stop() 
{
    aoiComponent.Stop();
    UDPServer::Stop();
}