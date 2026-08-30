#include "AOIComponent.hpp"

void AOIComponent::Initiailze()
{
    dispatcher.Initialize();
    factory.Initialize();
}

bool AOIComponent::MakeWorkers()
{
    //element.Print("AOI MakeWorkers");
    for (uint32_t i = 0; i < threadCount; i++)
    {
        std::unique_ptr<AOIEventWorker> worker =
            std::make_unique<AOIEventWorker>(element, i);
        if (worker == nullptr) return false;
        processPool.AddElement(std::move(worker));
    }
    return true;
}