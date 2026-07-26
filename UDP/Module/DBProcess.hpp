#ifndef DBPROCESS_H
#define DBPROCESS_H

#include "PacketProcess.hpp"



struct DBContext
{
    BasicContext& general;
    RedisControl& redis; 

    DBContext(BasicContext& general, RedisControl& redis):
        general(general), redis(redis)
    {}
};

class DBProcessDispatcher : public IPacketProcessDispatcher<DBContext>
{
public:
    using DBHandler = PacketResult(DBProcessDispatcher::*)(NetElement&, DBContext&);
private:
    // 보통 기능별로 따로 Service officer object를 따로 만듦.
    static std::array<DBHandler, ChangeToUINT(PacketType::LastDummy)> handlers; 

    RedisControl* GetRedis(uint32_t ID);
    PacketResult InsertToken(NetElement& param, DBContext& context);
public:
    DBProcessDispatcher():  IPacketProcessDispatcher() {}
    ~DBProcessDispatcher() {}

    bool Initialize() override;
    PacketResult Dispatch(NetElement& element, DBContext& context) override;
};




#endif