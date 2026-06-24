#ifndef PACKETPROCESS_H
#define PACKETPROCESS_H

#include <unordered_map>
#include <functional>

#include <iostream>
#include "Router.hpp"


enum class SERVER_ERROR
{
    SUCCESS,

    NULL_METHOD,

    INVALID_PACKET,

    CANNOT_FOUND_CLIENT,
    
    PARAMERTER_ERROR
};


template <typename MapKey, typename FuncElement>
class IPacketProcess
{
    std::unordered_map<MapKey, std::function<SERVER_ERROR(FuncElement&)>> func_map;
    
protected:

    Router& router;
    SessionManager& sessionManager;
    bool EmplaceFuncion(const MapKey key, std::function<SERVER_ERROR(FuncElement&)> func);
    
public:
    IPacketProcess():
        router(Router::GetInstance()),
        sessionManager(SessionManager::GetInstance()) 
    {}
    virtual ~IPacketProcess() 
    {
        func_map.clear();
    }
    virtual bool Initialize() = 0;
    std::function<SERVER_ERROR(FuncElement&)> GetFunc(const MapKey& key);
};

class PacketProcess : public IPacketProcess<PacketType, NetElement>
{
    bool FindSession(NetElement& param);
    
    SERVER_ERROR NULL_PACKET_METHOD(NetElement& param);
    SERVER_ERROR HelloNewSession(NetElement& param);

public:
    PacketProcess(): IPacketProcess() {}
    ~PacketProcess() {}

    bool Initialize() override;
    std::function<SERVER_ERROR(NetElement&)> GetFunc(const PacketType& key);
};





#endif