#include "Session.hpp"

Room::Room(int roomID, int maxClientsNum) :
    isAlive(true),
    roomID(roomID), maxClientsNum(maxClientsNum), currClientNum(0)
{
    room.reserve(maxClientsNum);
}
Room::~Room()
{

}

bool Room::AddClient(LinuxSession* client)
{
    if (isAlive.load() == false || client == nullptr || currClientNum == maxClientsNum) return false;

    auto pair = room.emplace(client->ID, client);

    if (!pair.second) return false;

    currClientNum++;
    return true;
}
LinuxSession* Room::FindClient(int id)
{
    LinuxSession* found = nullptr;
    if (isAlive == false) return found;
    auto it = room.find(id);

    if (it != room.end())
    {
        found = it->second;
    }
    return found;
}

bool Room::DeleteClient(int id)
{
    if (isAlive.load() == false) return false;
    auto it = room.find(id);
    if (it == room.end()) return false;
    
    //delete it->second;
    //it->second = nullptr;

    room.erase(id);
    currClientNum--;
    return true;
}
bool Room::DeleteClient(const LinuxSession& client)
{
    return DeleteClient(client.ID);
}


void Room::Destroy()
{
    isAlive.store(false);
    for (auto it = room.begin(); it != room.end();)
    {
        //delete it->second;
        //it->second = nullptr;

        it = room.erase(it);
    }
}


SessionManager* SessionManager::instance = nullptr;


bool SessionManager::AddSessionInBasicMap(std::unique_ptr<LinuxSession> session_ptr)
{
    if (session_ptr == nullptr) return false;
    std::lock_guard<std::mutex> lock(allSessionMutex);
    int id = nextID.load();
    session_ptr->ID = id;
    allSessions.emplace(session_ptr->ID, std::move(session_ptr));
    nextID.fetch_add(1);
    return true;
}

TokenValue SessionManager::GenerateUDPToken()
{
    // 하드웨어 엔트로피를 사용하는 난수 생성기 (가장 강력함)
    std::random_device rd;
        
    // 만약 성능이 중요하고 충분히 예측 불가능한 시드가 필요하다면 Mersenne Twister 추천
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(100000000, 999999999); // 9자리 난수 예시

    return dis(gen);
}

bool SessionManager::DeleteSessionInBasicMap(int id)
{
    std::lock_guard<std::mutex> lock(allSessionMutex);
    auto it = allSessions.find(id);
    if (it == allSessions.end()) return false;
    allSessions.erase(it);
    return true;
}


LinuxSession* SessionManager::FindSessionInAllSession(const uint32_t id)
{
    std::lock_guard<std::mutex> lock(allSessionMutex);
    auto it = allSessions.find(id);
    if (it == allSessions.end()) return nullptr;
    return it->second.get();
}

bool SessionManager::PendDelete(LinuxSession* session)
{
    if (session == nullptr) return false;
    session->isPendingDelete = true;
    return deletedSessionList.Push(session);   
}


void SessionManager::Work()
{
    while (isRunning)
    {
        CheckHeartBeats();
        DeleteSessionLoop();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void SessionManager::CheckHeartBeats()
{
    std::lock_guard<std::mutex> lock(allSessionMutex);

    auto now = std::chrono::steady_clock::now();
    //auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    const std::chrono::seconds timeoutDuration(30); // 30초 동안 하트비트 없으면 끊음

    for (auto it = allSessions.begin(); it != allSessions.end(); )
    {
        LinuxSession* session = it->second.get();
        if (session == nullptr) 
        {
            it++;
            continue;
        }

        // 세션이 이미 종료 절차를 밟고 있는 경우는 스킵
        if (session->isPendingDelete) 
        {
            it++;
            continue;
        }

        // [핵심] 현재 시간과 마지막 하트비트 시간 비교
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session->lastHeartbeatTime);
        if (elapsed > timeoutDuration)
        {
            PendDelete(session);
            // 3. 파이프라인(process pool / DB pool)에 "이 세션 끊어졌으니 컨텐츠 정리해!"라고 알림
        }
        it++;
    }

}
void SessionManager::DeleteSessionLoop()
{
    std::vector<LinuxSession*> temp;
    deletedSessionList.Swap(temp);

    for (auto it = temp.begin(); it != temp.end();)
    {
        //std::lock_guard<std::mutex> lock(deleteSessionMutex);
        LinuxSession* session = *it;
        if (session != nullptr && session->refCount == 0) 
        {
            DeleteSessionInBasicMap(session->ID);
            temp.erase(it);
        }
        else it++;
    }
    if (!temp.empty())
    {
        deletedSessionList.PushBackVector(std::move(temp));
    }
}

void SessionManager::Destroy()
{
    for (auto it = allSessions.begin(); it != allSessions.end();)
    {
        //delete it->second;
        //it->second = nullptr;

        it = allSessions.erase(it);
    }
}