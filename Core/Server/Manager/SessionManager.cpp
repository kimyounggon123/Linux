#include "SessionManager.hpp"
uint32_t SessionManager::timeOut = 30;

void SessionManager::CheckHeartBeats()
{
    auto now = std::chrono::steady_clock::now();
    //auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    const std::chrono::seconds timeoutDuration(timeOut); // n초 동안 하트비트 없으면 끊음

    std::vector<BasicSession*>& tempSessions = allSessions.GetObjects();
    for (auto it = tempSessions.begin(); it != tempSessions.end(); )
    {
        BasicSession* session = *it;

        // 무시 조건
        if (session == nullptr || session->IsHeartbeatEnabled() == false || session->IsPendDelete()) 
        {
            it++;
            continue;
        }

        // [핵심] 현재 시간과 마지막 하트비트 시간 비교
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - session->GetHeartBeatTime());
        if (elapsed > timeoutDuration)
        {
            PendDelete(session);
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
        if (session != nullptr) // && session->GetRefCount() == 0) 
        {
            uint32_t ID = session->GetID();
            DeleteSessionInBasicMap(session->GetID());
            temp.erase(it);
            LogTool::Log("SessionManager", "Erase ID " + std::to_string(ID));
        }
        else it++;
    }
    if (!temp.empty())
    {
        size_t size = temp.size();   
        deletedSessionList.PushChunk(temp, size);
    }
}

bool SessionManager::AddSessionInBasicMap(std::unique_ptr<BasicSession> session_ptr)
{
    if (session_ptr == nullptr) return false;
    int id = nextID.load();
    session_ptr->SetID(id);
    allSessions.AddElement(session_ptr->GetID(), std::move(session_ptr));
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
    return allSessions.Delete(id);
}

BasicSession* SessionManager::FindSession(const uint32_t id)
{
    BasicSession* found = allSessions.Find(id);
    if (found == nullptr || found->IsPendDelete()) return nullptr;
    return found;
}

bool SessionManager::PendDelete(BasicSession* session)
{
    if (session == nullptr) return false;
    session->PendDeleting();
    PendDeleteExtraProcess(session);
    deletedSessionList.Push(session);   
    return true;
}
bool SessionManager::PendDelete(const uint32_t id)
{
    return PendDelete(FindSession(id));
}
void SessionManager::PendDeleteAllSession()
{   
    auto& vec = allSessions.GetObjects();
    for (auto session : vec)
    {   
        session->PendDeleting();
        PendDeleteExtraProcess(session);
        deletedSessionList.Push(session);
    }
}

void SessionManager::Process()
{
    CheckHeartBeats();
    DeleteSessionLoop();
}