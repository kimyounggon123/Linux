#include "DBProcess.hpp"


bool DBProcess::Initialize()
{
    handlers.fill(&DBProcess::NULL_PACKET_METHOD);
    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &DBProcess::InsertToken;

    return true;
}
PROCESS_RESULT DBProcess::Dispatch(DBProcessElement& element)
{
    return (this->*handlers[element.pk->GetTypeUINT()])(element);
}

PROCESS_RESULT DBProcess::InsertToken(DBProcessElement& param)
{
    if (param.redis == nullptr) return PROCESS_RESULT::PARAMERTER_ERROR;
    std::string key = "30005";
    std::string value = "1234";
    
    if (param.redis->Exist(key)) return PROCESS_RESULT::CANNOT_FOUND_CLIENT;
    param.redis->Set(key, value, 10);

    OptionalString resultvalue = param.redis->Get(key);
    std::cout << "key: " << key << " value: " <<  resultvalue.value() << std::endl;
    param.redis->Delete(key);
    return PROCESS_RESULT::SUCCESS;
}
