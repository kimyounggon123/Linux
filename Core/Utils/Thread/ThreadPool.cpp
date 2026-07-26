#include "ThreadPool.hpp"

// while loop method 
void BasicThreadPoolElement::WorkThread()
{
    Work();
}
void BasicThreadPoolElement::Work()
{

}


bool BasicThreadPoolElement::Initialize()
{
    workerThread = std::thread(&BasicThreadPoolElement::WorkThread, this);
    isRunning = true;
    return true;
}

void BasicThreadPoolElement::Destroy()
{
    if (workerThread.joinable()) workerThread.join(); 
    isRunning = false;
}

bool ThreadPool::AddElement(std::unique_ptr<BasicThreadPoolElement> worker)
{
    if (worker == nullptr) return false;
    pool.push_back(std::move(worker));
    return true;
}

#include <iostream>
void ThreadPool::StopAll()
{
    for (auto& worker : pool)
    {
        if (worker == nullptr) continue;
        worker->Stop();
    }
}
void ThreadPool::Destroy()
{
    pool.clear();
}
    