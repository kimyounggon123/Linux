#ifndef NETWORKTASKWORKERCOMPONENT_H
#define NETWORKTASKWORKERCOMPONENT_H

#include "../Utils/Thread/ThreadPool.hpp"
#include "Pipe/PipePool.hpp"
#include "Workers/TaskWorker.hpp"

class NetworkTaskWorkerComponent 
{
    ThreadPool processPool;
    PipePool<NetworkTask> requestPool;
    PipePool<NetworkTask> responsePool;
public:
    NetworkTaskWorkerComponent(uint32_t poolSize, size_t waitms): 
        requestPool(poolSize * 2, waitms), responsePool(poolSize, waitms) 
    {}
    ~NetworkTaskWorkerComponent()
    {
        processPool.Stop("TaskWorker");
    }

    bool InsertProcessWorker(std::unique_ptr<BasicThreadPoolElement> worker) { return processPool.AddElement(std::move(worker)); }

    PipePool<NetworkTask>* GetRequestPool() {return &requestPool;}
    PipePool<NetworkTask>* GetResponsePool() {return &responsePool;}

    bool PushRequest(uint32_t key, const NetworkTask& task) {return requestPool.Push(key, task);}
    bool PushRequest(uint32_t key, NetworkTask&& task) {return requestPool.Push(key, std::move(task));}
    bool PopRequestChunk(uint32_t key, std::vector<NetworkTask>& tasks, size_t chunkSize) { return requestPool.PopChunk(key, tasks, chunkSize); }

    bool PushResponse(uint32_t key, const NetworkTask& task) {return responsePool.Push(key, task);}
    bool PushRespons(uint32_t key, NetworkTask&& task) {return responsePool.Push(key, std::move(task));}
    bool PopResponseChunk(uint32_t key, std::vector<NetworkTask>& tasks, size_t chunkSize) { return responsePool.PopChunk(key, tasks, chunkSize); }


};  

#endif