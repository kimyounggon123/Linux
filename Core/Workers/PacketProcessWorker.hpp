#ifndef PACKETPROCESSTHREADPOOL_H
#define PACKETPROCESSTHREADPOOL_H

#include "../Dispatcher/NetworkTask.hpp"
#include "../Dispatcher/CoreDispatcher.hpp"
#include "../Utils/Thread/ThreadPool.hpp"

class PacketProcessWorker : public BasicThreadPoolElement
{
private:
    CoreServices& services;
    CoreDispatcher& dispatcher;
    std::vector<NetworkTask> elementList;

    void Work() override;
public:
    PacketProcessWorker(CoreServices& services, CoreDispatcher& dispatcher, uint32_t ID = 0):
        BasicThreadPoolElement(ID),
        services(services),
        dispatcher(dispatcher)
    {}

    ~PacketProcessWorker() 
    {}
};





#endif