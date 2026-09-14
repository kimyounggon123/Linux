#ifndef UDPSESSION_H
#define UDPSESSION_H

#include "../Session/BasicSession.hpp"
//#include "../../Game/Entity/Player.hpp"

struct Endpoint
{
    uint32_t ip;
    uint16_t port;

    Endpoint(const sockaddr_in& addr): port(ntohs(addr.sin_port))
    {
        std::memcpy(&ip, &addr.sin_addr, sizeof(uint32_t));
    }
    bool operator==(const Endpoint& other) const
    {
        return ip == other.ip && port == other.port;
    }
};

class UDPSession : public BasicSession
{

    static constexpr uint32_t MaxSequenceDiff = 64;

    uint32_t sequence_count;
    Endpoint endpoint;
    //Player* connectedPlayer;

    
public:
    // void*      context_data;   
    UDPSession(const ProtocolType& type, const ConnectState& state, const sockaddr_in& addr):
        BasicSession(type, state, addr, true), 
        sequence_count(0),
        endpoint(addr)
    {}

    UDPSession(const UDPSession& other) = delete;
    //UDPSession& operator=(const UDPSession& other) = delete;
    
    ~UDPSession()
    {}

    int32_t GetDiffBetweenCurrentSequence(uint32_t incoming)
    {
        return static_cast<int32_t>(incoming - sequence_count);
    }

    void CheckSequence(uint32_t sequenceInPacket) 
    {
        int32_t diff = GetDiffBetweenCurrentSequence(sequenceInPacket);
        
        if (diff <= 0)
        {
            return;
        }

        if (diff > MaxSequenceDiff)
        {
            return;
        }
        sequence_count = sequenceInPacket;
    }

    //void IncreaseSequenceCount() {sequence_count++;}
    const uint32_t GetSequenceCount() const {return sequence_count;}
    const Endpoint& GetEndpoint() const {return endpoint;}
    void SetEndPoint(const sockaddr_in& addr)
    {
        SetAddr(addr);
        endpoint = {addr};
    }
};

#endif