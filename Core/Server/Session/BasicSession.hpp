#ifndef BASICSESSION_H
#define BASICSESSION_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <vector>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <chrono>
#include <cstring>

enum class ConnectorType : uint8_t { Basic, Admin, OtherServer };
enum class ProtocolType : uint8_t {TCP, UDP, UNIX};
enum class ConnectState : uint8_t { NONE, CONNECT, PEND_DISCONNECT, DISCONNECT };

using TokenValue = uint32_t;
class BasicSession
{
    static constexpr uint8_t MaxBanCount = 5;
    static constexpr uint32_t MaxDiffSequenceCount = 30;

    //ClientType clientType;
    ProtocolType protocolType;
    ConnectState currConnectState;

    uint32_t ID;

    std::atomic<uint16_t> refCount;
    
    // 하트비트 체크
    bool heartbeatEnabled;
    std::chrono::steady_clock::time_point lastHeartbeatTime;

    // 삭제 플래그
    bool isPendingDelete; 
    std::atomic<uint8_t> banCount;
protected:
    sockaddr_in addr;

public:
    BasicSession(const ProtocolType& type, const ConnectState& state, const struct sockaddr_in& addr, bool heartbeatEnabled):
        protocolType(type), currConnectState(state), 
        ID(0), addr(addr),
        refCount(0),
        heartbeatEnabled(heartbeatEnabled),
        lastHeartbeatTime(std::chrono::steady_clock::now()),
        isPendingDelete(false), banCount(0)
    {}
    BasicSession(const BasicSession&) = delete;
    BasicSession& operator=(const BasicSession&) = delete;
    
    virtual ~BasicSession() {}    

    // Getter , Setter
    const uint32_t GetID() const {return ID;}
    void SetID(const uint32_t& other) {ID = other;} 

    const sockaddr_in& GetAddr() const {return addr;}
    void SetAddr(const sockaddr_in& addr) {this->addr = addr;}
    
    const uint16_t GetRefCount() const {return refCount.load();}
    void RefThis() {refCount.fetch_add(1);}
    void ReleaseThisRef() {refCount.fetch_add(-1);} 

    bool IsHeartbeatEnabled() {return heartbeatEnabled;}
    void UpdateHeartbeat() { lastHeartbeatTime = std::chrono::steady_clock::now(); }
    const std::chrono::steady_clock::time_point GetHeartBeatTime() const {return lastHeartbeatTime;}

    const bool IsPendDelete() const {return isPendingDelete;}
    void PendDeleting() {isPendingDelete = true;}

    const ProtocolType GetProtocolType() const {return protocolType;}
    const ConnectState GetConnectState() const {return currConnectState;}
    void SetConnectState(const ConnectState& changeThis) {currConnectState = changeThis;}

}; 
#endif