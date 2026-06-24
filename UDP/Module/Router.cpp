#include "Router.hpp"
Router* Router::instance = nullptr;


bool Router::Initialize(uint32_t sessionPoolNum, uint32_t eachPacketCountInPool, uint32_t processPoolNum)
{

    if (processPoolNum == 0 || sessionPoolNum == 0 || recvToProcess.size() != 0 || processToSend.size() != 0) return false;
    for (int i = 0; i < sessionPoolNum; i++)
    {
        ThreadSafePool<PacketWithOwner> pkPool; 
        for (int i = 0; i < eachPacketCountInPool; i++)
        {
            std::unique_ptr<PacketWithOwner> packetWithOwner = std::make_unique<PacketWithOwner>(&pkPool);
            if (packetWithOwner == nullptr) 
            {
                return false;
            }
            pkPool.AddElement(std::move(packetWithOwner));
        }
        packetPoolList.push_back(std::move(pkPool)); 
    }
    
    
    for (int i = 0; i < processPoolNum; i++)
    {
        SessionPipe pipe(1000); 
        recvToProcess.push_back(std::move(pipe)); 
    }
    
    for (int i = 0; i < sessionPoolNum; i++)
    {
        SessionPipe pipe(1000); 
        processToSend.push_back(std::move(pipe)); 
    }

    return true;
}


bool Router::EnqueueSession(const PipeID& ID, NetElement session, uint32_t hashKey)
{
    std::vector<SessionPipe>* pipeList = nullptr;
    if (ID == PipeID::RecvToProcess) pipeList = &recvToProcess;
    else if (ID == PipeID::ProcessToSend) pipeList = &processToSend;
    else return false;

    // 만약 vector size가 2^n일 경우 사용 가능
    if (pipeList->empty()) return false;
    uint32_t key = hashKey & (pipeList->size() - 1);

    //uint32_t key = hashKey % recvToProcess.size();
    return (*pipeList)[key].enqueue(session);
}

bool Router::DequeueSession(const PipeID& ID, NetElement& session, uint32_t hashKey)
{
    std::vector<SessionPipe>* pipeList = nullptr;
    if (ID == PipeID::RecvToProcess) pipeList = &recvToProcess;
    else if (ID == PipeID::ProcessToSend) pipeList = &processToSend;
    else return false;

    // 만약 vector size가 2^n일 경우 사용 가능
    if (pipeList->empty()) return false;
    uint32_t key = hashKey & (pipeList->size() - 1);

    //uint32_t key = hashKey % recvToProcess.size();
    return (*pipeList)[key].dequeue(session);
}


bool Router::EnqueueDBProcess(DBBasicElement element, uint32_t hashKey)
{
    uint32_t key = hashKey & (processToDB.size() - 1);
    //uint32_t key = hashKey % recvToProcess.size();
    return processToDB[key].enqueue(element);
}
bool Router::DequeueDBProcess(DBBasicElement& element, uint32_t hashKey)
{
    uint32_t key = hashKey & (processToDB.size() - 1);
    //uint32_t key = hashKey % recvToProcess.size();
    return processToDB[key].dequeue(element);
}

bool Router::ReturnPacket(PacketWithOwner* withOwner)
{
    //uint32_t key = ID & (packetPoolList.size() - 1);
    return withOwner->ReturnToOwner();
}
bool Router::BorrowPacket(PacketWithOwner*& withOwner, uint32_t ID)
{
    uint32_t key = ID & (packetPoolList.size() - 1);
    return packetPoolList[ID].Pop(withOwner);
}