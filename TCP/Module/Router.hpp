#ifndef PIPEHUB_H
#define PIPEHUB_H

#include "NetStruct.hpp"
enum class PipeID
{
    RecvToProcess,
    ProcessToSend,
    ProcessToDB
};

using SessionPipe = ThreadSafeQueue<LinuxSession*>;
using DBPipe = ThreadSafeQueue<DBBasicElement>;
class Router
{
    bool isInitialized;
    static Router* instance;

    std::vector<std::unique_ptr<SessionPipe>> recvToProcess; // process pool 개수만큼 맞춤
    std::vector<std::unique_ptr<SessionPipe>> processToSend; // sender pool 개수만큼 맞춤

    std::vector<std::unique_ptr<DBPipe>> processToDB;        // process pool보단 적게 만듦. 보통 recv/send pool과 갯수를 맞춤
    std::vector<std::unique_ptr<DBPipe>> dbToProcess;

    Router() {}
    
public:
    static Router& GetInstance()
    {
        if (instance == nullptr) instance = new Router;
        return *instance;
    }
    static void DeleteInstance()
    {
        if (instance != nullptr) 
        {
            delete instance;
            instance = nullptr;
        }
    }
    ~Router() 
    {
        recvToProcess.clear();
        processToSend.clear();
    }

    bool Initialize(uint32_t processPoolNum, uint32_t senderPoolNum);


    bool EnqueueSession(const PipeID& ID, LinuxSession* session, uint32_t hashKey);
    bool DequeueSession(const PipeID& ID, LinuxSession*& session, uint32_t hashKey);

    bool EnqueueDBProcess(DBBasicElement element, uint32_t hashKey);
    bool DequeueDBProcess(DBBasicElement& element, uint32_t hashKey);
};
#endif