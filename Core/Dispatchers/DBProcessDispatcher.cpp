#include "DBProcessDispatcher.hpp"
std::array<DBProcessDispatcher::DBHandler, ChangeToUINT(PacketType::LastDummy)> DBProcessDispatcher::handlers;
bool DBProcessDispatcher::Initialize()
{
    if (isInitialized) return false;

    handlers.fill(&DBProcessDispatcher::NULL_PACKET_METHOD);
    
    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &DBProcessDispatcher::Test;

    isInitialized = true;
    return true;
}
