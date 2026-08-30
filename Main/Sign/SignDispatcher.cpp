#include "SignDispatcher.hpp"



bool SignDispatcher::Initialize()
{
    if (isInitialized) return false;

    EmplaceProcess(ChangeToUINT(PacketType::HelloNewClient), &SignDispatcher::LogIn);
    
    isInitialized = true;
    return true;
}


PacketResult SignDispatcher::LogIn(NetworkTask& task, SignUtilEx& utils)
{

    return PacketResult::Success;
}   