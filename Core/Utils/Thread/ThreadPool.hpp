#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <vector>
#include <thread>
#include <memory>
#include <chrono>


#include <iostream>
class BasicThreadPoolElement
{
    std::thread workerThread;
    void WorkThread();

protected:

    bool isRunning;
    uint32_t shardID;
    uint64_t lastHeartbeat;

    // while loop method 
    virtual void Work() {};
public:
    BasicThreadPoolElement(uint32_t shardID): shardID(shardID), isRunning(false), lastHeartbeat(0)
    {}
    virtual ~BasicThreadPoolElement() 
    {
        //std::cout << "[Thread] Stop Complete: [ID]"  << GetID() << std::endl;
    }

    virtual bool Initialize();
    void FreeThread() { isRunning = false; if (workerThread.joinable()) workerThread.join(); }
    bool IsDead() {return isRunning == false;}
    uint32_t GetShardID() const {return shardID;}
};



class ThreadPool
{
    std::vector<std::unique_ptr<BasicThreadPoolElement>> pool;

public:
    ThreadPool() {}
    ~ThreadPool() 
    {
        pool.clear();
    }

    bool AddElement(std::unique_ptr<BasicThreadPoolElement> worker);
    void FreeAllThread();
    const size_t Size() const {return pool.size();} 
};
#endif