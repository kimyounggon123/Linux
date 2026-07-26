#include "DBProcess.hpp"
std::array<DBProcessDispatcher::DBHandler, ChangeToUINT(PacketType::LastDummy)> DBProcessDispatcher::handlers; 
bool DBProcessDispatcher::Initialize()
{
    handlers.fill(&DBProcessDispatcher::NULL_PACKET_METHOD);
    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &DBProcessDispatcher::InsertToken;

    return true;
}
PacketResult DBProcessDispatcher::Dispatch(NetElement& element, DBContext& context)
{
    return (this->*handlers[element.pk->GetTypeUINT()])(element, context);
}

PacketResult DBProcessDispatcher::InsertToken(NetElement& param, DBContext& context)
{
    std::string key = "30005";
    std::string value = "1234";
            
    if (context.redis.Exist(key)) return PacketResult::INVALID_CLIENT;
    context.redis.Set(key, value, 10);

    OptionalString resultvalue = context.redis.Get(key);
    if (resultvalue.has_value())
    {
        std::cout << "key: " << key << " value: " <<  resultvalue.value() << std::endl;
        context.redis.Delete(key);
    }
    else
    {
        std::cout << "Key not found." << std::endl; 
    }

    return PacketResult::Success;
}
