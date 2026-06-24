#ifndef NETSTRUCT_H
#define NETSTRUCT_H

#include "Session.hpp"

struct NetElement
{
    LinuxSession* session;
    PacketWithOwner* pkWithOwner;

    NetElement(): session(nullptr), pkWithOwner(nullptr) {}
    NetElement(LinuxSession* session, PacketWithOwner* pkWithOwner):
        session(session), pkWithOwner(pkWithOwner) 
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