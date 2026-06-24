#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <vector>
#include <thread>
#include <memory>


class BasicThreadPoolElement
{
    std::thread workerThread;
    void WorkThread();

protected:
    uint32_t ID;
    bool isRunning;
    // while loop method 
    virtual void Work();

public:
    BasicThreadPoolElement(uint32_t ID): ID(ID), isRunning(false) {}
    virtual ~BasicThreadPoolElement() 
    {
        Destroy();
    }

    virtual bool Initialize();
    void Stop() {isRunning = false;}
    bool IsDead() {return isRunning == false;}
    virtual void Destroy();
};



class ThreadPool
{
    std::vector<std::unique_ptr<BasicThreadPoolElement>> pool;

public:
    ThreadPool() {}
    ~ThreadPool() 
    {
        Destroy();
    }

    bool AddElement(std::unique_ptr<BasicThreadPoolElement> worker);
    void StopAll();
    void Destroy();

    const size_t Size() const {return pool.size();} 
};
#endif