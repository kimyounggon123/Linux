#ifndef PIPEHUB_H
#define PIPEHUB_H

#include "NetStruct.hpp"

enum class PipeID
{
    RecvToProcess,
    ProcessToSend,
    ProcessToDB
};


using SessionPipe = ThreadSafeContainor<NetElement>;
using DBPipe = ThreadSafeContainor<DBBasicElement>;
class Router
{
    bool isInitialized;
    static Router* instance;

    std::vector<ThreadSafePool<PacketWithOwner>> packetPoolList;
    
    std::vector<SessionPipe> recvToProcess; // process pool 개수만큼 맞춤
    std::vector<SessionPipe> processToSend; // sender pool 개수만큼 맞춤

    std::vector<DBPipe> processToDB;        // process pool보단 적게 만듦. 보통 recv/send pool과 갯수를 맞춤
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

    bool Initialize(uint32_t sessionPoolNum, uint32_t eachPacketCountInPool, uint32_t processPoolNum);


    bool EnqueueSession(const PipeID& ID, NetElement session, uint32_t hashKey);
    bool DequeueSession(const PipeID& ID, NetElement& session, uint32_t hashKey);

    bool EnqueueSessionChunk(const PipeID& ID, std::vector<NetElement>&& session, uint32_t hashKey);
    bool DequeueSessionChunk(const PipeID& ID, std::vector<NetElement>& session, uint32_t hashKey);


    bool EnqueueDBProcess(DBBasicElement element, uint32_t hashKey);
    bool DequeueDBProcess(DBBasicElement& element, uint32_t hashKey);

    bool ReturnPacket(PacketWithOwner* withOwner);
    bool BorrowPacket(PacketWithOwner*& withOwner, uint32_t ID);
};
#endif