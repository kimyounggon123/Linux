#ifndef NETSTRUCT_H
#define NETSTRUCT_H

#include "Session.hpp"
#include "UtilsLinker.hpp"
#include <chrono>

struct NetElement
{
    struct sockaddr_in addr;
    LinuxSession* session;
    Packet* pk;

    std::chrono::time_point<std::chrono::high_resolution_clock> recv_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> process_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> send_time;

    NetElement(): 
        addr{},
        session(nullptr), pk(nullptr),
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now())
    {
        if (session != nullptr) session->AddRef();
    }
    NetElement(struct sockaddr_in addr, LinuxSession* session, Packet* pk):
        addr(addr),
        session(session), pk(pk),
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now())
    {
        if (session != nullptr) session->AddRef();
    }

    void RecordProcessStartTime()
    {
        process_start_time = std::chrono::high_resolution_clock::now();
    }
    void RecordSendTime()
    {
        send_time = std::chrono::high_resolution_clock::now();
    }

    std::chrono::duration<double, std::milli> GetDurationBetweenRecvAndProcess()
    {
        return process_start_time - recv_time;
    }
    std::chrono::duration<double, std::milli> GetDurationBetweenProcessAndSend()
    {
        return send_time - process_start_time;
    }


    virtual ~NetElement() 
    {
        if (session != nullptr) session->Release();
    }
};

struct DBProcessElement : public NetElement
{
    RedisControl* redis;

    DBProcessElement():
        NetElement(),
        redis(nullptr)
    {}
        
    DBProcessElement(struct sockaddr_in addr, LinuxSession* session,  Packet* pk):
        NetElement(addr, session, pk),
        redis(nullptr)
    {}

    ~DBProcessElement()
    {
        //if (session != nullptr) session->Release();
    }

    void Clear() 
    {
        redis = nullptr;
    }
};
#endif