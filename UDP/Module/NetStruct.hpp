#ifndef NETSTRUCT_H
#define NETSTRUCT_H

#include "Session.hpp"
#include "UtilsLinker.hpp"
struct PacketWithOwner
{
    ThreadSafePool<PacketWithOwner>* owner;
    Packet pk;  

    PacketWithOwner(): owner(nullptr) {}
    PacketWithOwner(ThreadSafePool<PacketWithOwner>* owner): owner(owner) {}

    bool ReturnToOwner()
    {
        if (owner == nullptr) return false;
        owner->Push(this);
        return true;
    }
};

struct NetElement
{
    struct sockaddr_in addr;
    LinuxSession* session;
    PacketWithOwner* pkWithOwner;

    NetElement(): addr{}, session(nullptr), pkWithOwner(nullptr) {}
    NetElement(LinuxSession* session, PacketWithOwner* pkWithOwner):
        addr{}, session(session), pkWithOwner(pkWithOwner) 
    {}
    NetElement(const sockaddr_in& addr, LinuxSession* session, PacketWithOwner* pkWithOwner):
        addr(addr), session(session), pkWithOwner(pkWithOwner) 
    {}
    
    ~NetElement() = default;
};

#include "UtilsLinker.hpp"
enum class DBProcessID
{
    None,
    UDPconnect
};

struct DBBasicElement
{
    DBProcessID processID;
    
    LinuxSession* session;
    uint32_t userDBID;
    TokenValue token;

    RedisControl* redis;

    DBBasicElement():
        processID(DBProcessID::None),
        session(nullptr), userDBID(0), token(0),
        redis(nullptr)
        {}
        
    DBBasicElement(DBProcessID processID, LinuxSession* session, uint32_t userDBID, TokenValue token):
        processID(processID),
        session(session), userDBID(userDBID), token(token),
        redis(nullptr)
        {}

    virtual ~DBBasicElement() = default;

    virtual void Clear() 
    {
        processID = DBProcessID::None;
        session = nullptr;
        userDBID = 0;
        token = 0;
    }
};


#endif