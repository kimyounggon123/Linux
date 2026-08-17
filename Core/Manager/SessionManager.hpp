#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <random>
#include "../Session/BasicSession.hpp"
#include "IManager.hpp"
#include "../Utils/Thread/ThreadPool.hpp"
#include "../Utils/Thread/ThreadSafeContainor.hpp"

class SessionManager : public IManager
{
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
    ~SessionManager() 
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


#endif