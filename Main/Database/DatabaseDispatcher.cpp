#include "DatabaseDispatcher.hpp"

PacketResult DatabaseDispatcher::LogIn(NetworkTask& task, DatabaseUtils& utils)
{
    

    return PacketResult::Success;
}

bool DatabaseDispatcher::Initialize()
{
    if (isInitialized) return false;
    EmplaceProcess(ChangeToUINT(PacketType::HelloNewClient), &DatabaseDispatcher::LogIn);
    isInitialized = true;
    return true;
}