#include "SignDispatcher.hpp"

bool SignDispatcher::Initialize()
{
    if (isInitialized) return false;

    if (!EmplaceProcess(ChangeToUINT(PacketType::HelloNewClient), &SignDispatcher::LogIn)) return false;
    
    isInitialized = true;
    return true;
}

PacketResult SignDispatcher::LogIn(NetworkTask& task, SignUtilEx& utils)
{
    //std::cout << "hello" << std::endl;

    // NetworkTask toSendDB = task;
    // toSendDB.pk = utils.pkPool->Acquire();
    // if (toSendDB.pk == nullptr) return PacketResult::RESOURCE_FAMINE;
    // toSendDB.pk->CopyOther(task.pk);
    // utils.toSendDB->Push(task.session->GetID(), std::move(toSendDB));
    return PacketResult::Success;
}   