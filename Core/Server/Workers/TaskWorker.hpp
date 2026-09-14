#ifndef TASKWORKER_H
#define TASKWORKER_H

#include "../../Utils/Thread/ThreadPool.hpp"
#include "../NetworkTask.hpp"
#include "../CoreServices.hpp"
#include "../Dispatcher/INetworkTaskDispatcher.hpp"

// template <typename Task>
// class BaseTaskProcessWorker : public BasicThreadPoolElement
// {
//     void Work() override
//     {
//         BasicSession* session = nullptr;
//         PacketResult result;
//         Packet* pk = nullptr;
//         while(isRunning)
//         {
//             if (!PopTasks(32)) continue;
//             Dispatch();
//             tasks.clear();
//         }
//     }

//     virtual bool PopTasks(size_t chunkSize) = 0;
//     virtual void Dispatch() = 0;

// protected:
//     CoreServices services;
//     std::vector<Task> tasks;
// public:
//     BaseTaskProcessWorker(const CoreServices& services, uint32_t ID = 0): BasicThreadPoolElement(ID), services(services) {}
//     ~BaseTaskProcessWorker() {}
//     virtual bool Initialize() {return BasicThreadPoolElement::Initialize();}
// };


template <typename DispatcherName, typename Utils>
class NetworkTaskProcessWorker : public BasicThreadPoolElement
{
    CoreServices services;
    std::vector<NetworkTask> tasks;
    Utils utils;
    INetworkTaskDispatcher<DispatcherName, Utils>& dispatcher;

    bool PopTasks()
    {
        return services.processPipePool->PopChunk(shardID, tasks, 32);
    }

        void Dispatch() 
    {
        Packet* pk = nullptr;
        PacketResult result = PacketResult::Try;

        for (auto& task : tasks)
        {
            pk = task.pk;
            if (pk == nullptr) continue;

            task.RecordProcessStartTime(); // 로직 시간 측정
            result = dispatcher.Dispatch(pk->GetTypeUINT(), task, utils); // 실제 패킷 로직 처리
            task.RecordProcessEndTime(); // 로직 시간 측정                
            
            if (task.nextStage == ElementStage::Send) // 일반적인 패킷 처리
            {
                task.pk->SetResult(result); // 패킷 결과 처리
                services.sendPipePool->Push(shardID, std::move(task)); // Send Pool로 전송
            }
            else
            {
                services.pkPool->Release(task.pk);
            }
        }
    }
    
    void Work() override
    {
        BasicSession* session = nullptr;
        PacketResult result;
        Packet* pk = nullptr;
        while(isRunning)
        {
            if (!PopTasks()) continue;
            Dispatch();
            tasks.clear();
        }
    }

public:
    NetworkTaskProcessWorker(const CoreServices& services, const Utils& utils, 
        INetworkTaskDispatcher<DispatcherName, Utils>& dispatcher,
        uint32_t ID = 0): 
        BasicThreadPoolElement(ID), services(services), utils(utils), dispatcher(dispatcher)
    {}
    ~NetworkTaskProcessWorker() {}
};




// class PacketProcessWorker : public BasicThreadPoolElement
// {
// private:
//     CoreServices& services;
//     std::vector<NetworkTask> elementList;
//     void Work() override;
// public:
//     PacketProcessWorker(CoreServices& services, uint32_t ID = 0):
//         BasicThreadPoolElement(ID),
//         services(services),
//         dispatcher(dispatcher) {}

//     ~PacketProcessWorker() {}
// };





#endif