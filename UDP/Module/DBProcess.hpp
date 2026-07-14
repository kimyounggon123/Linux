#ifndef DBPROCESS_H
#define DBPROCESS_H

#include "PacketProcess.hpp"


class DBProcess : public IPacketProcess<PacketType, DBProcessElement>
{
public:
    using DBHandler = PROCESS_RESULT(DBProcess::*)(DBProcessElement&);
private:
    // 보통 기능별로 따로 Service officer object를 따로 만듦.
    inline static std::array<DBHandler, ChangeToUINT(PacketType::LastDummy)> handlers; 

    RedisControl* GetRedis(uint32_t ID);
    PROCESS_RESULT InsertToken(DBProcessElement& param);
public:
    DBProcess():  IPacketProcess() {}
    ~DBProcess() {}

    bool Initialize() override;
    PROCESS_RESULT Dispatch(DBProcessElement& element);
};




#endif