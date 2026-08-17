#ifndef PIPEPOOL_H
#define PIPEPOOL_H

#include "../Dispatcher/NetworkTask.hpp"
#include "../Utils/Thread/ThreadSafeContainor.hpp"
#include "../Utils/Containor/Containors.hpp"

template <typename T>
class PipePool
{
public:
    using Pipe = ThreadSafeQueue<T>;
private:
    PoolUsingKey<Pipe> pipePool;
    ThreadSafeQueue<T>* GetPipe(const uint32_t shardKey) { return pipePool.GetElement(shardKey); }

public:
    // poolSize는 반드시 2^n 크기로 잡으세요.
    PipePool(uint32_t poolSize = 8, size_t waitMs = 200)
    {
        std::unique_ptr<ThreadSafeQueue<T>> pipe = nullptr;
        for (uint32_t i = 0; i < poolSize; i++)
        {
            pipe = std::make_unique<ThreadSafeQueue<T>>(waitMs);
            pipePool.AddElement(std::move(pipe));
        }
    }

    // 복사 금지.
    PipePool(const PipePool<T>&) = delete;
    PipePool& operator=(const PipePool&) = delete;

    ~PipePool() 
    {}

    bool Push(const uint32_t key, T&& push)
    {
        ThreadSafeQueue<T>* pipe = GetPipe(key);
        if (pipe == nullptr) return false;
        pipe->enqueue(std::move(push));
        return true;
    }
    bool Pop(const uint32_t key, T& pop)
    {
        ThreadSafeQueue<T>* pipe = GetPipe(key);
        if (pipe == nullptr) return false;
        return pipe->dequeue(pop);
    }
    bool PopChunk(const uint32_t key, std::vector<T>& chunk, const size_t maxSize = 32)
    {
        ThreadSafeQueue<T>* pipe = GetPipe(key);
        if (pipe == nullptr) return false;
        return pipe->dequeue_chunk(chunk, maxSize);
    }
};

using NetWorkPipePool = PipePool<NetworkTask>;

#endif