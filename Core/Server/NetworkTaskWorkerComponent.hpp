#ifndef NETWORKTASKWORKERCOMPONENT_H
#define NETWORKTASKWORKERCOMPONENT_H

#include "../Utils/Thread/ThreadPool.hpp"
#include "../Utils/Containor/ThreadSafeContainor.hpp"
#include "Workers/TaskWorker.hpp"
#include "CoreServices.hpp"

class NetworkTaskWorkerComponent 
{
    ThreadPool processPool;
    NetWorkPipePool requestPipePool;
    NetWorkPipePool responsePipePool;
    BroadcastTaskPipePool broadcastPool; // 따로 Task Pool이 있음.
public:
    NetworkTaskWorkerComponent(uint32_t poolSize, size_t waitMsRequest, size_t waitMsResponse, size_t waitMsBroadcast): 
        requestPipePool(poolSize * 2, waitMsRequest), responsePipePool(poolSize, waitMsResponse), broadcastPool(poolSize, waitMsBroadcast)
    {}
    ~NetworkTaskWorkerComponent()
    {
        Stop();
    }

    bool InsertProcessWorker(std::unique_ptr<BasicThreadPoolElement> worker) { return processPool.AddElement(std::move(worker)); }
    void Start() {processPool.Start("Task Worker");}
    void Stop() {processPool.Stop("Task Worker");}

    NetWorkPipePool* GetRequestPipePool() {return &requestPipePool;}
    NetWorkPipePool* GetResponsePipePool() {return &responsePipePool;}
    BroadcastTaskPipePool* GetBroadcastPipePool() {return &broadcastPool;}

    // bool PushRequest(uint32_t key, const NetworkTask& task) {return requestPipePool.Push(key, task);}
    // bool PushRequest(uint32_t key, NetworkTask&& task) {return requestPipePool.Push(key, std::move(task));}
    // bool PopRequestChunk(uint32_t key, std::vector<NetworkTask>& tasks, size_t chunkSize) { return requestPipePool.PopChunk(key, tasks, chunkSize); }

    // bool PushResponse(uint32_t key, const NetworkTask& task) {return responsePipePool.Push(key, task);}
    // bool PushRespons(uint32_t key, NetworkTask&& task) {return responsePipePool.Push(key, std::move(task));}
    // bool PopResponseChunk(uint32_t key, std::vector<NetworkTask>& tasks, size_t chunkSize) { return responsePipePool.PopChunk(key, tasks, chunkSize); }


};  

#endif