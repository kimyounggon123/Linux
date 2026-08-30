#include "SessionManagerUDP.hpp"

void SessionManagerUDP::PendDeleteExtraProcess(BasicSession* session)
{
    if (session == nullptr || session->GetProtocolType() != ProtocolType::UDP) return;
    UDPSession* change = dynamic_cast<UDPSession*>(session);
    udpMap.Delete(change->GetEndpoint().ip);
}

bool SessionManagerUDP::AddUDPSession(std::unique_ptr<UDPSession> session_ptr)
{
    udpMap.Add(session_ptr->GetEndpoint().ip, session_ptr.get());
    return AddSessionInBasicMap(std::move(session_ptr));
}

UDPSession* SessionManagerUDP::FindUDP(const Endpoint& endpoint)
{
    UDPSession* retval = nullptr;
    if (!udpMap.Find(endpoint.ip, retval)) return nullptr;
    return retval;
}