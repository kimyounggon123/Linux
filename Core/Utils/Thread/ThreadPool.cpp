#include "ThreadPool.hpp"

// while loop method 
void BasicThreadPoolElement::WorkThread()
{
    Work();
}

bool BasicThreadPoolElement::Initialize()
{
    workerThread = std::thread(&BasicThreadPoolElement::WorkThread, this);
    isRunning = true;
    return true;
}

bool ThreadPool::AddElement(std::unique_ptr<BasicThreadPoolElement> worker)
{
    if (worker == nullptr) return false;
    pool.push_back(std::move(worker));
    return true;
}

void ThreadPool::FreeAllThread()
{
    for (auto& worker : pool)
    {
        if (worker == nullptr) continue;
        worker->FreeThread();
        std::cout << "[ThreadPool] Stop Complete: [ID]"  << worker->GetShardID() << std::endl;
    }
    pool.clear();
}
