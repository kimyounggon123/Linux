#ifndef NETELEMENT_H
#define NETELEMENT_H

#include <chrono>
#include "Packet/Packet.hpp"
#include "Session/BasicSession.hpp"

enum class ElementStage : uint8_t
{
    Send,
    DropThis,
    LAST_DUMMY
};

struct NetworkTask
{
    static constexpr uint32_t typeCount = ChangeToUINT(PacketType::LAST_DUMMY);
    ElementStage nextStage;
    BasicSession* session;
    Packet* pk;

    std::chrono::time_point<std::chrono::high_resolution_clock> recv_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> process_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> process_end_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> db_start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> db_end_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> send_time;

    NetworkTask():
        nextStage(ElementStage::Send),
        session(nullptr), pk(nullptr), 
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        process_end_time(std::chrono::high_resolution_clock::now()),
        db_start_time(std::chrono::high_resolution_clock::now()),
        db_end_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now())
    {
        if (session != nullptr) session->RefThis();
    }

    NetworkTask(ElementStage stage, BasicSession* session, Packet* pk):
        nextStage(stage),
        session(session), pk(pk),
        recv_time(std::chrono::high_resolution_clock::now()),
        process_start_time(std::chrono::high_resolution_clock::now()),
        process_end_time(std::chrono::high_resolution_clock::now()),
        db_start_time(std::chrono::high_resolution_clock::now()),
        db_end_time(std::chrono::high_resolution_clock::now()),
        send_time(std::chrono::high_resolution_clock::now())
    {
        if (session != nullptr) session->RefThis();
    }

    NetworkTask(const NetworkTask& other):
        nextStage(other.nextStage),
        session(other.session), pk(other.pk),
        recv_time(other.recv_time),
        process_start_time(other.process_start_time),
        process_end_time(other.process_end_time),
        db_start_time(other.db_start_time),
        db_end_time(other.db_end_time),
        send_time(other.send_time)
    {
        if (session != nullptr) session->RefThis();   
    }

    ~NetworkTask() 
    {
        if (session != nullptr) session->ReleaseThisRef();
    }

    void RecordProcessStartTime() { process_start_time = std::chrono::high_resolution_clock::now(); }
    void RecordProcessEndTime() {process_end_time = std::chrono::high_resolution_clock::now();}
    void RecordDBStartTime() { db_start_time = std::chrono::high_resolution_clock::now(); }
    void RecordDBEndTime() { db_end_time = std::chrono::high_resolution_clock::now();}
    void RecordSendTime() { send_time = std::chrono::high_resolution_clock::now(); }

    std::chrono::duration<double, std::milli> GetDurationBetweenRecvAndProcess() { return process_start_time - recv_time; }
    std::chrono::duration<double, std::milli> GetProcessDuration() { return process_end_time - process_start_time; }
    std::chrono::duration<double, std::milli> GetDBDuration() { return db_end_time - db_start_time; }
    std::chrono::duration<double, std::milli> GetDurationBetweenProcessAndSend() { return send_time - process_end_time; }
    std::chrono::duration<double, std::milli> GetPipelineDuration() { return send_time - recv_time; }


    void GoToHere(const ElementStage& next) {nextStage = next;}

    void ShowTimeStamp(bool flagOnlyWarning)
    {
        double betweenRecvAndProcessStart = GetDurationBetweenRecvAndProcess().count() > 0.0 ? GetDurationBetweenRecvAndProcess().count() : 0.0;
        double processDuration = GetProcessDuration().count() > 0.0 ? GetProcessDuration().count() : 0.0;
        double dbDuration = GetDBDuration().count() > 0.0 ? GetDBDuration().count() : 0.0;
        double betweenProcessAndSend = GetDurationBetweenProcessAndSend().count() > 0.0 ? GetDurationBetweenProcessAndSend().count() : 0.0;
        double pipelineDuration = GetPipelineDuration().count() > 0.0 ? GetPipelineDuration().count() : 0.0;

        if (!flagOnlyWarning || betweenRecvAndProcessStart > 2.0) std::cout << "Between recv and process: " << betweenRecvAndProcessStart << " (ms)" << std::endl;
        if (!flagOnlyWarning || processDuration > 1.0) std::cout << "Process Duration: " << processDuration << " (ms)" << std::endl;
        if (!flagOnlyWarning || dbDuration > 3.0) std::cout << "DB Duration: " << dbDuration << " (ms)" << std::endl;
        if (!flagOnlyWarning || betweenProcessAndSend > 2.0) std::cout << "Between process and send: " << betweenProcessAndSend << " (ms)" << std::endl;
        if (!flagOnlyWarning || pipelineDuration > 10.0) std::cout << "Pipeline Duration: " << pipelineDuration << " (ms)\n" << std::endl;
           
    }
};


#endif