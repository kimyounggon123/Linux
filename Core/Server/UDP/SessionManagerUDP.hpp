#ifndef SESSIONMANAGERUDP_H
#define SESSIONMANAGERUDP_H

#include "../Manager/SessionManager.hpp"
#include "UDPSession.hpp"


/*
class SessionManager : public IManager
{
    static uint32_t sessionTimeOut; // 이 이상 지나면 연결 끊기

    std::atomic<uint32_t> nextID;
    std::atomic<uint32_t> currClientNum;

    ThreadElementRegistry<uint32_t, BasicSession> allSessions;
    ThreadSafeContainor<BasicSession*> deletedSessionList;
    //auto FindSessionBasicMapIterator(const uint32_t id);
    
    void CheckHeartBeats();
    void DeleteSessionLoop();
    bool DeleteSessionInBasicMap(uint32_t id);

public:
    SessionManager() : nextID(0), currClientNum(0) {}
    virtual ~SessionManager() 
    {
        deletedSessionList.Clear();   
    }

    // 1. 일단 allSession에 등록
    bool AddSessionInBasicMap(std::unique_ptr<BasicSession> session_ptr);
    bool PendDelete(BasicSession* session);
    bool PendDelete(const uint32_t id);
    TokenValue GenerateUDPToken();
    BasicSession* FindSession(const uint32_t id);

    void Process() override
    {
        CheckHeartBeats();
        DeleteSessionLoop();
    }
};

*/


using UDPSearcher = ThreadSafeMap<uint32_t, UDPSession*>;

class SessionManagerUDP : public SessionManager
{
    UDPSearcher udpMap; 
    void PendDeleteExtraProcess(BasicSession* session) override;
public:
    SessionManagerUDP(){}
    ~SessionManagerUDP(){}
    bool AddUDPSession(std::unique_ptr<UDPSession> session_ptr);
    UDPSession* FindUDP(const Endpoint& endpoint);
};


#endif