#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <random>
#include "IManager.hpp"
#include "../Session/BasicSession.hpp"

class SessionManager : public IManager
{
    static uint32_t timeOut; // 이 이상 지나면 연결 끊기

    std::atomic<uint32_t> nextID;
    ThreadElementRegistry<uint32_t, BasicSession> allSessions;
    ThreadSafeContainor<BasicSession*> deletedSessionList;
    //auto FindSessionBasicMapIterator(const uint32_t id);
    
    void CheckHeartBeats();
    void DeleteSessionLoop();
    bool DeleteSessionInBasicMap(uint32_t id);
protected:
    virtual void PendDeleteExtraProcess(BasicSession* session) {}
public:
    SessionManager() : nextID(0) {}
    virtual ~SessionManager() 
    {
        deletedSessionList.Clear();   
    }

    // 1. 일단 allSession에 등록
    bool AddSessionInBasicMap(std::unique_ptr<BasicSession> session_ptr);
    bool PendDelete(BasicSession* session);
    bool PendDelete(const uint32_t id);

    BasicSession* FindSession(const uint32_t id);
    TokenValue GenerateUDPToken();

    void PendDeleteAllSession();
    void Process() override;

    size_t GetSessionCount() {return allSessions.GetSize();}
};


#endif