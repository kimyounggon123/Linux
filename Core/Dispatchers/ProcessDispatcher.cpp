#include "ProcessDispatcher.hpp"

std::array<GeneralProcessDispatcher::GeneralHandler, ChangeToUINT(PacketType::LastDummy)> GeneralProcessDispatcher::handlers;
bool GeneralProcessDispatcher::Initialize()
{
    if (isInitialized) return false;
    handlers.fill(&GeneralProcessDispatcher::NULL_PACKET_METHOD);

    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &GeneralProcessDispatcher::Test; 
    isInitialized = true;
    return true;
}

