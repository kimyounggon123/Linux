#ifndef SESSION_H
#define SESSION_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <vector>
#include <algorithm>
#include <unordered_map>
#include <atomic>
#include <memory>

#include "UtilsLinker.hpp"

enum class ClientType : uint8_t
{
    BASIC,
    ADMIN,
    OTHER_SERVER
};

enum class SessionState : uint8_t
{
    NONE,

    CONNECTED,
    INGAME,
    DISCONNECTING
};

using TokenValue = uint32_t;
struct LinuxSession
{
    //ClientType clientType;
    SessionState state;

    uint32_t ID;
    sockaddr_in addr;

    bool try_udp_flag;
    TokenValue udp_token;

    // 작업 풀
    static constexpr uint8_t maxBanCount = 5;
    std::atomic<uint8_t> banCount;

    // 하트비트 체크
    std::chrono::steady_clock::time_point lastHeartbeatTime;
    //std::atomic<uint64_t> lastHeartbeatTime;
    
    // 삭제 플래그
    bool isPendingDelete; 
    std::atomic<uint32_t> refCount;

    // void*      context_data;   
    LinuxSession(int socket_fd) :
        state(SessionState::CONNECTED),
        ID(0), addr{},
        try_udp_flag(false), udp_token(0),
        banCount(0),
        lastHeartbeatTime(std::chrono::steady_clock::now()),
        isPendingDelete(false), refCount(0)
    {}

    ~LinuxSession()
    {
        Clear();
    }

    bool Start()
    {
        return true;
    }

    void Clear()
    {
        state = SessionState::NONE;
    }

    void UpdateHeartbeat() 
    {
        lastHeartbeatTime = std::chrono::steady_clock::now(); 
    }

    
    void AddRef() {refCount.fetch_add(1);}
    void Release() {refCount.fetch_add(-1);}
};



class Room
{
    std::atomic<bool> isAlive;
    std::atomic<int> currClientNum;

    int roomID;
    int maxClientsNum;

protected:
    std::unordered_map<int, LinuxSession*> room;
    virtual void Update() {}

public:
    Room(int roomID, int maxClientNum);
    virtual ~Room();

    bool AddClient(LinuxSession* client);
    LinuxSession* FindClient(int id);

    bool DeleteClient(int id);
    bool DeleteClient(const LinuxSession& client);

    void Destroy();
};


class RoomManager
{
    std::atomic<int> next_id;
    std::atomic<int> currRoomNums;
    std::unordered_map<int, std::unique_ptr<Room>> rooms;

public:
};

class SessionManager : public BasicThreadPoolElement
{
    std::atomic<uint32_t> nextID;
    std::atomic<int> currClientNum;

    std::mutex allSessionMutex;
    std::unordered_map<uint32_t, std::unique_ptr<LinuxSession>> allSessions;

    ThreadSafeContainor<LinuxSession*> deletedSessionList;

    static SessionManager* instance;
    SessionManager() : BasicThreadPoolElement(0), nextID(0), currClientNum(0) {}

    auto FindSessionBasicMapIterator(const uint32_t id);

    void Work() override;

    void CheckHeartBeats();
    void DeleteSessionLoop();
    bool DeleteSessionInBasicMap(int id);

    void Destroy() override;
public:
    static SessionManager& GetInstance()
    {
        if (instance == nullptr) instance = new SessionManager;
        return *instance;
    }
    static void DeleteInstance()
    {
        if (instance == nullptr) return;
        delete instance;
        instance = nullptr;
    }
    ~SessionManager() 
    {
        //Destroy();
    }

    // 1. 일단 allSession에 등록
    bool AddSessionInBasicMap(std::unique_ptr<LinuxSession> session_ptr, uint32_t key);
    bool PendDelete(LinuxSession* session);
    TokenValue GenerateUDPToken();
    LinuxSession* FindSessionInAllSession(const uint32_t id);
};

#endif