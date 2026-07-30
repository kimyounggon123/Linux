#ifndef AOIPIPE_H
#define AOIPIPE_H

#include "../../Core/Pipe/Router.hpp"
#include "../Data/GameData.hpp"
using AOIPipe = ThreadSafeQueue<GameTask>;
class AOIPipePool
{
    PoolUsingKey<AOIPipe> aoiPipePool;

    AOIPipe* GetPipe(const uint32_t shardKey)
    {
        return aoiPipePool.GetElement(shardKey);
    }
public:
    AOIPipePool(){}
    ~AOIPipePool(){}

    bool Initialize(uint32_t threadCount);
    bool PushGameTask(const uint32_t shardKey, GameTask&& task);
    bool PopGameTaskChunk(const uint32_t shardKey, std::vector<GameTask>& tasks, const size_t maxSize = 32);
};
#endif