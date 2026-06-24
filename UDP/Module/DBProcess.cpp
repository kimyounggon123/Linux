#include "DBProcess.hpp"


SERVER_ERROR DBProcess::FindSessionTokenIntoRedis(DBBasicElement& param)
{
    if (param.redis == nullptr) return SERVER_ERROR::PARAMERTER_ERROR;
    std::string tokenSTR = param.redis->IntToString(param.token);
    
    if (!param.redis->Exist(tokenSTR))
    {
        return SERVER_ERROR::CANNOT_FOUND_CLIENT;
    }

    //sessionManager.AddSessionInBasicMap

    return SERVER_ERROR::SUCCESS;
}

bool DBProcess::Initialize()
{
    EmplaceFuncion(DBProcessID::UDPconnect, 
        [this](DBBasicElement& element) {return this->FindSessionTokenIntoRedis(element);});

    return true;
}