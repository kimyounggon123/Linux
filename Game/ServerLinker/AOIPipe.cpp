#include "AOIPipe.hpp"

bool AOIPipePool::Initialize(const uint32_t threadCount)
{
    for (uint32_t i = 0; i < threadCount; i++)
    {

    }
    return true;
}
bool AOIPipePool::PushGameTask(const uint32_t shardKey, GameTask&& task)
{
    AOIPipe* pipe = GetPipe(shardKey);
    if (pipe == nullptr) return false;
    return pipe->enqueue(std::move(task));
}

bool AOIPipePool::PopGameTaskChunk(const uint32_t shardKey, std::vector<GameTask>& tasks, const size_t maxSize)
{
    AOIPipe* pipe = GetPipe(shardKey);
    if (pipe == nullptr) return false;
    return pipe->dequeue_chunk(tasks, maxSize);
}