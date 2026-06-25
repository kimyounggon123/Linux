#ifndef PACKETPROCESS_H
#define PACKETPROCESS_H

#include <unordered_map>
#include <functional>

#include <iostream>
#include "Session.hpp"
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
    SERVER_ERROR NULL_PACKET_METHOD(FuncElement& param)
    {
        return SERVER_ERROR::NULL_METHOD;
    }
    bool EmplaceFuncion(const MapKey key, std::function<SERVER_ERROR(FuncElement&)> func)
    {
        auto pair = func_map.emplace(key, func);
        return pair.second;
    } 
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
    std::function<SERVER_ERROR(FuncElement&)> GetFunc(const MapKey& key)
    {
        auto func = func_map.find(key);
        if (func != func_map.end()) return func->second;

        // 수상한 헤더 포착 시 에러 method 호출
        return [this](FuncElement& element){return this->NULL_PACKET_METHOD(element);};
    }
};

class PacketProcess : public IPacketProcess<PacketType, NetElement>
{
    bool FindSession(NetElement& param);


    SERVER_ERROR TryHelloNewSession(NetElement& param);
    SERVER_ERROR GetResultTryHelloNewSession(NetElement& param);

public:
    PacketProcess(): IPacketProcess() {}
    ~PacketProcess() {}

    bool Initialize() override;
    //std::function<SERVER_ERROR(NetElement&)> GetFunc(const PacketType& key);
};





#endif