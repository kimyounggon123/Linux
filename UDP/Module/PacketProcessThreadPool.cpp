#include "PacketProcessThreadPool.hpp"

void PacketProcessThreadElement::Work()
{
    while(isRunning)
    {
        NetElement element;
        if (!router.DequeueSession(PipeID::RecvToProcess, element, ID)) 
        {   
            continue;
        }

        auto func = process->GetFunc(element.pk->GetType<PacketType>());
        SERVER_ERROR error = func(element); 
        if (error != SERVER_ERROR::SUCCESS)
        {
            element.pk->SetResult(PacketResult::Fail);
        }

        router.EnqueueSession(PipeID::ProcessToSend, std::move(element), ID);
        //std::cout << "Go to Write part!" << std::endl;
    }
}



bool PacketProcessThreadElement::Initialize()
{
    if (process == nullptr) return false;
    return BasicThreadPoolElement::Initialize();
}
void PacketProcessThreadElement::Destroy()
{
    BasicThreadPoolElement::Destroy();
}