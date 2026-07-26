#include "SessionManager.hpp"

bool SessionManager::Initialize()
{
    return BasicThreadPoolElement::Initialize();
}

bool SessionManager::AddSessionInBasicMap(std::unique_ptr<BasicSession> session_ptr)
{
    if (session_ptr == nullptr) return false;
    std::lock_guard<std::mutex> lock(allSessionMutex);
    int id = nextID.load();
    session_ptr->SetID(id);
    allSessions.emplace(session_ptr->GetID(), std::move(session_ptr));
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

bool SessionManager::DeleteSessionInBasicMap(uint32_t id)
{
    std::lock_guard<std::mutex> lock(allSessionMutex);
    auto it = allSessions.find(id);
    if (it == allSessions.end()) return false;
    allSessions.erase(it);
    return true;
}


BasicSession* SessionManager::FindSessionInAllSession(const uint32_t id)
{
    std::lock_guard<std::mutex> lock(allSessionMutex);
    auto it = allSessions.find(id);
    if (it == allSessions.end()) return nullptr;
    return it->second.get();
}

bool SessionManager::PendDelete(BasicSession* session)
{
    if (session == nullptr) return false;
    session->PendDeleting();
    return deletedSessionList.Push(session);   
}


void SessionManager::Work()
{
    while (isRunning)
    {
        //printf("Check loop\n");
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
        BasicSession* session = it->second.get();
        if (session == nullptr) 
        {
            it++;
            continue;
        }

        // 세션이 이미 종료 절차를 밟고 있는 경우는 스킵
        if (session->IsPendDelete()) 
        {
            it++;
            continue;
        }

        // [핵심] 현재 시간과 마지막 하트비트 시간 비교
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session->GetHeartBeatTime());
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
    std::vector<BasicSession*> temp;
    deletedSessionList.Swap(temp);

    for (auto it = temp.begin(); it != temp.end();)
    {
        BasicSession* session = *it;
        if (session != nullptr && session->GetRefCount() == 0) 
        {
            uint32_t ID = session->GetID();
            DeleteSessionInBasicMap(session->GetID());
            temp.erase(it);
            //printf("Erased ID: %d\n", ID);
        }
        else it++;
    }
    if (!temp.empty())
    {
        size_t size = temp.size();   
        deletedSessionList.PushChunk(temp, size);
    }
}

void SessionManager::Destroy()
{
    for (auto it = allSessions.begin(); it != allSessions.end();)
    {
        it = allSessions.erase(it);
    }
}