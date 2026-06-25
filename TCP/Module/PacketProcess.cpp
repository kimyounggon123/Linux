#include "PacketProcess.hpp"
//std::unordered_map<PacketType, std::function<SERVER_ERROR(FuncParameter&)>> PacketProcess::func_map;

bool PacketProcess::Initialize()
{
    bool retval = true;
    retval = retval && EmplaceFuncion(PacketType::Default,
        [this](NetElement& element){return this->NULL_PACKET_METHOD(element);});
        
    return retval;
}

bool PacketProcess::FindSession(NetElement& param)
{
    if (param.session->state == SessionState::CONNECTED) return true;
    
    param.session = sessionManager.FindSessionInAllSession(param.pkWithOwner->pk.GetClientID());
    if (param.session == nullptr) return false;

    return true;
}
    
SERVER_ERROR PacketProcess::TryHelloNewSession(NetElement& param)
{   
    if (FindSession(param) == false) return SERVER_ERROR::CANNOT_FOUND_CLIENT;
    //param.session->address = param.addr;
    param.session->AddRef();

    param.session->udp_token = sessionManager.GenerateUDPToken();

    DBBasicElement element(DBProcessID::UDPconnect, param.session, 0, param.session->udp_token);
    router.EnqueueDBProcess(element, param.session->udp_token);

    param.pkWithOwner->pk.SetResult(PacketResult::Success);
    return SERVER_ERROR::SUCCESS;
}

SERVER_ERROR PacketProcess::GetResultTryHelloNewSession(NetElement& param)
{
    param.session->state = SessionState::CONNECTED;
    param.pkWithOwner->pk.SetResult(PacketResult::Success);

    param.session->Release();
    return SERVER_ERROR::SUCCESS;
}

