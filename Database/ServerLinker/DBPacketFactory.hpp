#ifndef DBPACKETFACTORY_H
#define DBPACKETFACTORY_H

#include <array>
#include "../../Core/Packet/IPacketFactory.hpp"
#include "../Dispatcher/DatabaseTask.hpp"

class DBPacketFactory : public IPacketFactory<DBPacketFactory, DatabaseTask, ChangeToUINT(DatabaseTaskType::LAST_DUMMY)>
{
    bool Test(Packet& pk, DatabaseTask& task);
public:
    DBPacketFactory(PacketPool& pkPool): IPacketFactory(pkPool){}
    ~DBPacketFactory() {}

    bool Initialize() override;
};

#endif