#include "Router.hpp"
Router* Router::instance = nullptr;


bool Router::Initialize(int pkNum)
{
    for (int i = 0; i < pkNum; i++)
    {
        std::unique_ptr<Packet> pk = std::make_unique<Packet>();
        PacketPool.AddElement(std::move(pk));
    }
    return true;
}

bool Router::EnqueueElement(PipeType ID, NetElement&& element)
{
    SessionPipe& pipe = ID == PipeType::ProcessInput ? GOTOprocessWorker : GOTOsender;
    return pipe.enqueue(std::move(element));
}

bool Router::DequeueElementAsChunk(PipeType ID, std::vector<NetElement>& chunk, size_t maxSize)
{
    SessionPipe& pipe = ID == PipeType::ProcessInput ? GOTOprocessWorker : GOTOsender;
    return pipe.dequeue_chunk(chunk, maxSize);
}

bool Router::EnqueueDBElement(NetElement&& element)
{
    return GOTODatabaseWorker.enqueue(std::move(element));
}
bool Router::DequeueDBElementAsChunk(std::vector<NetElement>& chunk, size_t maxSize)
{
    return GOTODatabaseWorker.dequeue_chunk(chunk, maxSize);
}
