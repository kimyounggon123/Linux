#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <random>

#include "BasicSession.hpp"
#include "../Utils/Thread/ThreadPool.hpp"
#include "../Utils/Thread/ThreadSafeContainor.hpp"

class SessionManager : public BasicThreadPoolElement
{
    std::atomic<uint32_t> nextID;
    std::atomic<int> currClientNum;

    std::mutex allSessionMutex;
    
    std::unordered_map<uint32_t, std::unique_ptr<BasicSession>> allSessions;
    ThreadSafeContainor<BasicSession*> deletedSessionList;

    //auto FindSessionBasicMapIterator(const uint32_t id);

    void Work() override;

    void CheckHeartBeats();
    void DeleteSessionLoop();
    bool DeleteSessionInBasicMap(uint32_t id);

    void Destroy() override;
public:
    SessionManager() : BasicThreadPoolElement(0), nextID(0), currClientNum(0) {}
    ~SessionManager() 
    {
        Destroy();
    }

    bool Initialize() override;
    // 1. 일단 allSession에 등록
    bool AddSessionInBasicMap(std::unique_ptr<BasicSession> session_ptr);
    bool PendDelete(BasicSession* session);
    TokenValue GenerateUDPToken();
    BasicSession* FindSessionInAllSession(const uint32_t id);
};


#endif