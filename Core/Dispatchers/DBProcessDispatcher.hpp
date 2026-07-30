#ifndef DBPROCESSDISPATCHER_H
#define DBPROCESSDISPATCHER_H


#include <functional>
#include <iostream>

#include "IProcessDispatcher.hpp"
#include "../../Parser/Database/RedisProtocol.hpp"
class DBProcessDispatcher : public IProcessDispatcher<NetworkTask, DBContext>
{
public: 
    using DBHandler = PacketResult(DBProcessDispatcher::*)(NetworkTask&, DBContext&);
private:
    static std::array<DBHandler, ChangeToUINT(PacketType::LastDummy)> handlers;
    bool isInitialized;

    // processer field 
    RedisProtocol redisHandler;
    PacketResult Test(NetworkTask& element, DBContext& context) {return redisHandler.Test(element, context);} 

public:
    DBProcessDispatcher() : IProcessDispatcher(), isInitialized(false) {}
    ~DBProcessDispatcher() {}

    PacketResult Dispatch(NetworkTask& element, DBContext& context) override
    {
        const auto type = element.pk->GetTypeUINT();
        if (type >= handlers.size()) return NULL_PACKET_METHOD(element, context);
        return (this->*handlers[type])(element, context);
    }
    bool Initialize() override;
};
#endif