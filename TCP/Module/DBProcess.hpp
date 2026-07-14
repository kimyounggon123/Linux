#ifndef DBPROCESS_H
#define DBPROCESS_H

#include "PacketProcess.hpp"


class DBProcess : public IPacketProcess<PacketType, NetElement>
{
public:
    using DBHandler = PacketResult(DBProcess::*)(NetElement&);
private:
    // 보통 기능별로 따로 Service officer object를 따로 만듦.
    inline static std::array<DBHandler, ChangeToUINT(PacketType::LastDummy)> handlers; 

    RedisControl* GetRedis(uint32_t ID);
    PacketResult InsertToken(NetElement& param);
public:
    DBProcess():  IPacketProcess() {}
    ~DBProcess() {}

    bool Initialize() override;
    PacketResult Dispatch(NetElement& element);
};




#endif