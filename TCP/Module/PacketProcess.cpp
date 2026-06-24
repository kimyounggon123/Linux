#include "PacketProcess.hpp"
//std::unordered_map<PacketType, std::function<SERVER_ERROR(FuncParameter&)>> PacketProcess::func_map;

template <typename MapKey, typename FuncElement>
bool IPacketProcess<MapKey, FuncElement>::EmplaceFuncion(const MapKey key, std::function<SERVER_ERROR(FuncElement&)> func)
{
    auto pair = func_map.emplace(key, func);
    return pair.second;
} 

template <typename MapKey, typename FuncElement>
std::function<SERVER_ERROR(FuncElement&)> IPacketProcess<MapKey, FuncElement>::GetFunc(const MapKey& key)
{
    auto func = func_map.find(key);
    if (func != func_map.end()) return func->second;

    // 수상한 헤더 포착 시 에러 method 호출
    return [this](FuncElement& element){return this->NULL_PACKET_METHOD(element);};
}

bool PacketProcess::Initialize()
{
    bool retval = true;
    retval = retval && EmplaceFuncion(PacketType::Default,
        [this](NetElement& element){return this->NULL_PACKET_METHOD(element);});
        
    return retval;
}

SERVER_ERROR PacketProcess::NULL_PACKET_METHOD(NetElement& param)
{
    std::cout << "YOU CALL THE NULL PACKET METHOD" << std::endl;
    return SERVER_ERROR::NULL_METHOD;
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

