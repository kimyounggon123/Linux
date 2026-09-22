#include "DatabaseDispatcher.hpp"

PacketResult DatabaseDispatcher::LogIn(NetworkTask* task, DatabaseUtils& utils)
{
    std::cout << "hello: " << task->session << std::endl;
    return PacketResult::Success;
}

bool DatabaseDispatcher::Initialize()
{
    if (isInitialized) return false;
    EmplaceProcess(ChangeToUINT(PacketType::HelloNewClient), &DatabaseDispatcher::LogIn);
    isInitialized = true;
    return true;
}