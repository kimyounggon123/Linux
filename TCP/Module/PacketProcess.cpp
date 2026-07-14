#include "PacketProcess.hpp"

bool PacketProcess::Initialize()
{
    // 일단 다 null method로 초기화!
    handlers.fill(&PacketProcess::NULL_PACKET_METHOD);

    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &PacketProcess::HelloNewSession;
    handlers[ChangeToUINT(PacketType::ByeClient)] = &PacketProcess::NULL_PACKET_METHOD;

    return true;
}


PacketResult PacketProcess::Dispatch(NetElement& element)
{
    return (this->*handlers[element.pk->GetTypeUINT()])(element);
}





//std::unordered_map<PacketType, std::function<SERVER_ERROR(FuncParameter&)>> PacketProcess::func_map;

// bool PacketProcess::Initialize()
// {
//     bool retval = true;
//     retval = retval && EmplaceFuncion(PacketType::Default,
//         [this](NetElement& element){return this->NULL_PACKET_METHOD(element);});
    
//     retval = retval && EmplaceFuncion(PacketType::HelloNewClient,
//         [this](NetElement& element){return this->TryHelloNewSession(element);});
        
//     return retval;
// }

bool PacketProcess::FindSession(NetElement& param)
{
    if (param.session->state == SessionState::CONNECTED) return true;
    
    param.session = sessionManager.FindSessionInAllSession(param.pk->GetClientID());
    if (param.session == nullptr) return false;

    return true;
}
    
PacketResult PacketProcess::HelloNewSession(NetElement& param)
{   
    if (FindSession(param) == false) return PacketResult::INVALID_CLIENT;

    // 작동 테스트용
    // param.session->udp_token = sessionManager.GenerateUDPToken();
    // uint32_t resultInt = 0;
    // std::string resultSTR = "";
    // float resultFloat = 0.0f;
    // param.pk->ReadInt32(resultInt);
    // param.pk->ReadStringUTF8(resultSTR);
    // param.pk->ReadFloat(resultFloat);
    std::cout << "Hello new Client!" << std::endl;
    param.GoToHere(ElementStage::Database);
    return PacketResult::Success;
}


