#include "DBComponent.hpp"

void DBComponent::Initialize()
{
    dispatcher.Initialize();
    factory.Initialize();
}
bool DBComponent::MakeWorkers(const std::string& addr)
{
    //element.Print("DB MakeWorkers");

    for (uint32_t i = 0; i < threadCount; i++)
    {
        std::unique_ptr<DBProcessWorker> worker = 
            std::make_unique<DBProcessWorker>(element, i, addr.c_str());
        if (worker == nullptr) return false;
        processPool.AddElement(std::move(worker));
    }
    return true;
}