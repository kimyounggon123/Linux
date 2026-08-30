#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <vector>
#include <thread>
#include <memory>
#include <chrono>
#include <iostream>


namespace ThreadUtil
{
    inline void SleepMs(uint32_t milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
    inline void Yield() {  std::this_thread::yield(); }
    inline std::thread::id GetThreadID() { return std::this_thread::get_id(); }
}

class BasicThreadPoolElement
{
protected:
    bool isRunning;
    uint32_t shardID;
    uint64_t lastHeartbeat;

    virtual void Work() {};
private:
    std::thread workerThread;
    void WorkThread();

public:
    BasicThreadPoolElement(uint32_t shardID): shardID(shardID), isRunning(false), lastHeartbeat(0)
    {}
    virtual ~BasicThreadPoolElement() 
    {}

    virtual bool Initialize() {return true;}
    void Start(const std::string& where = "None");
    void Stop(const std::string& where = "None");
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

    void Start(const std::string& where = "None")
    {
        for (auto it = pool.begin(); it != pool.end(); it++)
        {
            BasicThreadPoolElement* ptr = (*it).get();
            if (ptr == nullptr) 
            {
                continue;
            }
            ptr->Start(where);
        }
    }
    bool AddElement(std::unique_ptr<BasicThreadPoolElement> worker);
    const size_t Size() const {return pool.size();} 
    void Stop(const std::string& where = "None")
    {
        for (auto it = pool.begin(); it != pool.end(); it++)
        {
            BasicThreadPoolElement* ptr = (*it).get();
            ptr->Stop(where);
        }
    }
};

#endif