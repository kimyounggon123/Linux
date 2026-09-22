#ifndef AOICOMPONENT_H
#define AOICOMPONENT_H

#include "AOITaskWorker.hpp"

class AOIComponent
{
    uint32_t threadCount;
    ThreadPool threadPool;

    // AOI Worker에 넣을 구조
    AOITaskPool taskPool;
    AOITaskPipePool pipePool;
    AOIWorkDict dict;


public:
    AOIComponent(uint32_t threadCount): 
        threadCount(threadCount), taskPool(9000, 0), pipePool(threadCount, 200)
    {}    
    ~AOIComponent() {}

    bool Initialize();

    void Start() {threadPool.Start();}
    void Stop() {threadPool.Stop();}
};

#endif