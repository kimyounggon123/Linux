#ifndef AOITASKWORKER_H
#define AOITASKWORKER_H

#include "../../../Core/Utils/Thread/ThreadSafeContainor.hpp"
#include "../../../Core/Utils/Thread/ThreadPool.hpp"


class AOITaskWorker : public BasicThreadPoolElement
{   

    void Work() override;
public:
    AOITaskWorker(uint32_t shardID): BasicThreadPoolElement(shardID) {}
    ~AOITaskWorker() {}
};

#endif