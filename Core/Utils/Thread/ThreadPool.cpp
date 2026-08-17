#include "ThreadPool.hpp"

// while loop method 
void BasicThreadPoolElement::WorkThread()
{
    Work();
}

void BasicThreadPoolElement::Start(const std::string& where)
{
    if (isRunning) return;
    workerThread = std::thread(&BasicThreadPoolElement::WorkThread, this);
    isRunning = true;
    //printf("Start Thread %d [%s]\n", shardID, where.c_str());
}
void BasicThreadPoolElement::Stop(const std::string& where) 
{
    //printf("Delete Thread %d [%s]\n", shardID, where.c_str());
    isRunning = false; if (workerThread.joinable()) workerThread.join();
}
bool ThreadPool::AddElement(std::unique_ptr<BasicThreadPoolElement> worker)
{
    if (worker == nullptr) return false;
    pool.push_back(std::move(worker));
    return true;
}
