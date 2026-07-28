#ifndef UDPSESSION_H
#define UDPSESSION_H

#include "../../Core/Sessions/BasicSession.hpp"
#include "../../Game/Player.hpp"
class UDPSession : public BasicSession
{
    static constexpr uint32_t MaxSequenceDiff = 64;

    uint32_t sequence_count;
    uint64_t sessionKey;

    Player* connectedPlayer;
public:
    // void*      context_data;   
    UDPSession(const ProtocolType& type, const ConnectState& state, const sockaddr_in& addr):
        BasicSession(type, state, addr), 
        sequence_count(0), connectedPlayer(nullptr)
    {
        std::memcpy(&sessionKey, &addr, sizeof(sockaddr_in));
    }

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
    const uint64_t GetSessionKey() const {return sessionKey;}

    Player* GetConnectedPlayer() const {return connectedPlayer;}
    void ConnectPlayer(Player* player) {connectedPlayer = player;}
};

#endif