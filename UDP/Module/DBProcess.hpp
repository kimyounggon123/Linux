#ifndef DBPROCESS_H
#define DBPROCESS_H

#include "PacketProcess.hpp"
#include "Router.hpp"


class DBProcess : public IPacketProcess<DBProcessID, DBBasicElement>
{
    RedisControl* GetRedis(uint32_t ID);
    SERVER_ERROR FindSessionTokenIntoRedis(DBBasicElement& param);
public:
    DBProcess():  IPacketProcess() {}
    ~DBProcess() {}

    bool Initialize() override;
};




#endif