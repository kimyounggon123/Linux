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
    bool result = false;
    switch(ID)
    {
    case PipeType::ProcessInput:
        result = GOTOprocessWorker.enqueue(std::move(element));
        break;
    case PipeType::ProcessOutput:
        result = GOTOsender.enqueue(std::move(element));
        break;
    case PipeType::DBInput:
        result = GOTODatabaseWorker.enqueue(std::move(element));
        break;
    }
    return result;
}

bool Router::DequeueElementAsChunk(PipeType ID, std::vector<NetElement>& chunk, size_t maxSize)
{
    bool result = false;
    switch(ID)
    {
    case PipeType::ProcessInput:
        result = GOTOprocessWorker.dequeue_chunk(chunk, maxSize);
        break;
    case PipeType::ProcessOutput:
        result = GOTOsender.dequeue_chunk(chunk, maxSize);
        break;
    case PipeType::DBInput:
        result = GOTODatabaseWorker.dequeue_chunk(chunk, maxSize);
        break;
    }
    return result;
}
