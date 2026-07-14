#include "DBProcess.hpp"


bool DBProcess::Initialize()
{
    handlers.fill(&DBProcess::NULL_PACKET_METHOD);
    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &DBProcess::InsertToken;

    return true;
}
PacketResult DBProcess::Dispatch(NetElement& element)
{
    return (this->*handlers[element.pk->GetTypeUINT()])(element);
}

PacketResult DBProcess::InsertToken(NetElement& param)
{
    if (param.context.redis == nullptr) return PacketResult::PARAMETER_ERROR;

    std::string key = "30005";
    std::string value = "1234";
            
    if (param.context.redis->Exist(key)) return PacketResult::INVALID_CLIENT;
    param.context.redis->Set(key, value, 10);

    OptionalString resultvalue = param.context.redis->Get(key);
    if (resultvalue.has_value())
    {
        std::cout << "key: " << key << " value: " <<  resultvalue.value() << std::endl;
        param.context.redis->Delete(key);
    }
    else
    {
        std::cout << "Key not found." << std::endl; 
    }

    return PacketResult::Success;
}
