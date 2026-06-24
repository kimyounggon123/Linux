#include "DBProcess.hpp"


SERVER_ERROR DBProcess::InsertToken(DBBasicElement& param)
{
    if (param.redis == nullptr) return SERVER_ERROR::PARAMERTER_ERROR;
    std::string tokenSTR = param.redis->IntToString(param.token);
    std::string sessionIDSTR = param.redis->IntToString(param.session->ID);
    if (param.redis->Exist(tokenSTR)) return SERVER_ERROR::CANNOT_FOUND_CLIENT;
    param.redis->Set(tokenSTR, sessionIDSTR, 10);


    Packet* pk = nullptr;
    //param.session->processPool.Pop();
    return SERVER_ERROR::SUCCESS;
}

bool DBProcess::Initialize()
{
    EmplaceFuncion(DBProcessID::UDPconnect, 
        [this](DBBasicElement& element) {return this->InsertToken(element);});

    return true;
}