#include "PacketProcess.hpp"

bool PacketProcess::Initialize()
{
    // 일단 다 null method로 초기화!
    handlers.fill(&PacketProcess::NULL_PACKET_METHOD);

    handlers[ChangeToUINT(PacketType::HelloNewClient)] = &PacketProcess::TryHelloNewSession;
    handlers[ChangeToUINT(PacketType::ByeClient)] = &PacketProcess::NULL_PACKET_METHOD;

    return true;
}

PROCESS_RESULT PacketProcess::Dispatch(NetElement& element)
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
    
PROCESS_RESULT PacketProcess::TryHelloNewSession(NetElement& param)
{   
    if (FindSession(param) == false) return PROCESS_RESULT::CANNOT_FOUND_CLIENT;
    param.session->udp_token = sessionManager.GenerateUDPToken();

    PacketResult result = param.pk->GetResult<PacketResult>();  
    
    size_t offset = 0;
    uint32_t resultInt = 0;
    std::string resultSTR = "";
    float resultFloat = 0.0f;

    param.pk->ReadInt32(resultInt, offset);
    param.pk->ReadStringUTF8(resultSTR, offset);
    param.pk->ReadFloat(resultFloat, offset);

    //std::cout << "int: " << resultInt << " str: " << resultSTR  << " float: " << resultFloat << std::endl;
    
    param.pk->ClearBuffer();
    if (!param.pk->PushInt32(123212)) return PROCESS_RESULT::FAIL_INSERTING_DATA;
    if (!param.pk->PushStringUTF8("hi new client!")) return PROCESS_RESULT::FAIL_INSERTING_DATA;
    if (!param.pk->PushFloat(1819.123f)) return PROCESS_RESULT::FAIL_INSERTING_DATA;

    Packet* pk = nullptr;
    if (!router.PopPacket(pk)) return PROCESS_RESULT::RESOURCE_FAMINE;
    pk->CopyOther(param.pk);

    DBProcessElement dbElement = {param.session, pk};
    router.EnqueueDBElement(std::move(dbElement));

    return PROCESS_RESULT::SUCCESS;
}

// PROCESS_RESULT PacketProcess::GetResultTryHelloNewSession(NetElement& param)
// {
//     param.session->state = SessionState::CONNECTED;
//     param.pk->SetResult(PacketResult::Success);
//     return PROCESS_RESULT::SUCCESS;
// }



