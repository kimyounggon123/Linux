#include "Router.hpp"

bool Router::Initialize(int howManyShard)
{
    std::unique_ptr<SessionPipe> pipe = nullptr;
    for (int i = 0; i < howManyShard; i++)
    {
        pipe = std::make_unique<SessionPipe>(200);
        GOTOprocessWorker.AddElement(std::move(pipe));

        pipe = std::make_unique<SessionPipe>(200);
        GOTOsender.AddElement(std::move(pipe));

        pipe = std::make_unique<SessionPipe>(200);
        GOTODatabaseWorker.AddElement(std::move(pipe));
    }
    return true;
}



bool Router::EnqueueElement(PipeType ID, uint32_t shardKey, NetElement&& element)
{
    SessionPipe* pipe = GetPipe(ID, shardKey);
    if (pipe == nullptr) return false;
    return pipe->enqueue(std::move(element));
}

bool Router::DequeueElementAsChunk(PipeType ID, uint32_t shardKey, std::vector<NetElement>& chunk, size_t maxSize)
{
    SessionPipe* pipe = GetPipe(ID, shardKey);
    if (pipe == nullptr) return false;
    return pipe->dequeue_chunk(chunk, maxSize);
}
