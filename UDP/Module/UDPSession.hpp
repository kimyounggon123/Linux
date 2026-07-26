#ifndef UDPSESSION_H
#define UDPSESSION_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <atomic>
#include <memory>

#include "../../Public/BasicSession.hpp"

class UDPSession : public BasicSession
{
    static constexpr uint32_t MaxSequenceDiff = 64;

    //ClientType clientType;
    uint32_t sequence_count;
    uint64_t sessionKey;
public:
    // void*      context_data;   
    UDPSession(const ProtocolType& type, const ConnectState& state, const sockaddr_in& addr):
        BasicSession(type, state, addr), sequence_count(0)
    {
        std::memcpy(&sessionKey, &addr, sizeof(sockaddr_in));
    }

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
};

#endif