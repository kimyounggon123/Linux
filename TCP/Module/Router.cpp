#include "Router.hpp"
Router* Router::instance = nullptr;


bool Router::Initialize(uint32_t processPoolNum, uint32_t workerThreadPoolNum)
{
    if (processPoolNum == 0 || workerThreadPoolNum == 0 || recvToProcess.size() != 0 || processToSend.size() != 0) return false;
    for (int i = 0; i < processPoolNum; i++)
    {
        std::unique_ptr<SessionPipe> pipe = std::make_unique<SessionPipe>(1000); 
        recvToProcess.push_back(std::move(pipe)); 

        
    }
    
    for (int i = 0; i < workerThreadPoolNum; i++)
    {
        std::unique_ptr<SessionPipe> pipe = std::make_unique<SessionPipe>(1000); 
        processToSend.push_back(std::move(pipe)); 

        std::unique_ptr<DBPipe> dbPipe = std::make_unique<DBPipe>(1000); 
        processToDB.push_back(std::move(dbPipe));

        dbPipe = std::make_unique<DBPipe>(1000); 
        dbToProcess.push_back(std::move(dbPipe));
    }
        
    return true;
}


bool Router::EnqueueSession(const PipeID& ID, LinuxSession* session, uint32_t hashKey)
{
    std::vector<std::unique_ptr<SessionPipe>>* pipeList = nullptr;
    if (ID == PipeID::RecvToProcess) pipeList = &recvToProcess;
    else if (ID == PipeID::ProcessToSend) pipeList = &processToSend;
    else return false;

    // 만약 vector size가 2^n일 경우 사용 가능
    if (pipeList->empty()) return false;
    uint32_t key = hashKey & (pipeList->size() - 1);

    //uint32_t key = hashKey % recvToProcess.size();
    SessionPipe* pipe = (*pipeList)[key].get();
    return pipe->enqueue(session);
}

bool Router::DequeueSession(const PipeID& ID, LinuxSession*& session, uint32_t hashKey)
{
    std::vector<std::unique_ptr<SessionPipe>>* pipeList = nullptr;
    if (ID == PipeID::RecvToProcess) pipeList = &recvToProcess;
    else if (ID == PipeID::ProcessToSend) pipeList = &processToSend;
    else return false;

    // 만약 vector size가 2^n일 경우 사용 가능
    if (pipeList->empty()) return false;
    uint32_t key = hashKey & (pipeList->size() - 1);

    //uint32_t key = hashKey % recvToProcess.size();
    SessionPipe* pipe = (*pipeList)[key].get();
    return pipe->dequeue(session);
}


bool Router::EnqueueDBProcess(DBBasicElement element, uint32_t hashKey)
{
    uint32_t key = hashKey & (processToDB.size() - 1);
    //uint32_t key = hashKey % recvToProcess.size();
    return processToDB[key].get()->enqueue(element);
}
bool Router::DequeueDBProcess(DBBasicElement& element, uint32_t hashKey)
{
    uint32_t key = hashKey & (processToDB.size() - 1);
    //uint32_t key = hashKey % recvToProcess.size();
    return  processToDB[key].get()->dequeue(element);
}