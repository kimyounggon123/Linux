#include "AOIComponent.hpp"



bool AOIComponent::Initialize()
{
    if (!dict.Initialize()) return false;
    if (!taskPool.Initialize()) return false;

    std::unique_ptr<AOITaskWorker> worker = nullptr;
    for (uint32_t i = 0; i < threadCount; i++)
    {
        worker = std::make_unique<AOITaskWorker>(i, &taskPool, &pipePool, &dict);
        if (worker == nullptr) return false;
        threadPool.AddElement(std::move(worker));
    }

    return true;
}