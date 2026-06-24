#include "PacketProcessThreadPool.hpp"

void PacketProcessThreadElement::Work()
{
    LinuxSession* session = nullptr;
    std::vector<PacketWithOwner*> pkList;

    while(isRunning)
    {
        if (!router.DequeueSession(PipeID::RecvToProcess, session, ID)) 
        {   
            continue;
        }
        // 모든 패킷 받아오기.
        if (!session->processContainor.Swap(pkList)) continue;

        session->AddRef();  

        for (auto* pkWithOwner : pkList)
        {
            NetElement param(session, pkWithOwner);
            Packet& pk = pkWithOwner->pk;
            auto func = process->GetFunc(pk.GetType<PacketType>());
            SERVER_ERROR error = func(param); // 함수 내부에서 NetElement 사용. 이 때 Token을 UDP 서버에 전송하는 함수면 해당 함수 내부에서 복사되어 UDP 서버로 전송

            // 에러 메시지 처리 구간
            if (error != SERVER_ERROR::SUCCESS)
            {
                pk.SetResult(PacketResult::Fail);
            }   
        }
        session->sendContainor.PushBackVector(std::move(pkList));

        router.EnqueueSession(PipeID::ProcessToSend, std::move(session), ID);
        
        session->Release();
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