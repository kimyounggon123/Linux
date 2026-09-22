#include "SignServer.hpp"


bool SignServer::MakeTaskWorkers()
{
    if (!dispatcher.Initialize()) return false;
    if (!DBconnection.MakeSocket(LOCALHOST)) 
    {
        std::cout << "DBconnection MakeSocket Error" << std::endl;
        return false;
    }
    if (!DBconnection.MakeWorkers()) 
    {
        std::cout << "MakeWorkers Error" << std::endl;
        return false;
    }

    std::unique_ptr<SignTaskWorker> worker = nullptr;
    SignUtilEx utils = {services.pkPool, services.sessionManager, &toSendDB};
    for (uint32_t i = 0; i < threadPoolCount * 2; i++)
    {
        worker = std::make_unique<SignTaskWorker>(services, utils, dispatcher,  i);
        if (worker == nullptr) return false;
        taskWorkerComponent.InsertProcessWorker(std::move(worker));   
    }
    return true;
}

void SignServer::Start()
{
    DBconnection.Start();
    BaseServer::Start();
}
void SignServer::Stop() 
{
    BaseServer::Stop();
    DBconnection.Stop();
}