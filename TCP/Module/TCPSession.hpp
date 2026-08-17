#ifndef TCPSESSION_H
#define TCPSESSION_H

#include "../../Core/Session/BasicSession.hpp"
#include "../../Core/Utils/Buffer/Buffers.hpp"

class TCPSession : public BasicSession
{
    int socket_fd;       // Linux의 소켓 식별자 (File Descriptor)
    uint32_t epoll_events;    // 현재 epoll에 등록된 이벤트 플래그 (EPOLLIN, EPOLLOUT 등)

    bool try_udp_flag;
    TokenValue udp_token;

    RecvBuffer recv_buffer;
    SendBuffer send_buffer;

    void Destroy()
    {
        close(socket_fd);
        epoll_events = 0;
        //current_room = nullptr;
        recv_buffer.Clear();
        send_buffer.Clear();
    }
public:
    // void*      context_data;   
    TCPSession(int socket_fd, const ProtocolType& type, const ConnectState& state, const struct sockaddr_in& addr) :
        BasicSession(type, state, addr),
        socket_fd(socket_fd), epoll_events(EPOLLIN | EPOLLRDHUP | EPOLLERR),
        try_udp_flag(false), udp_token(0)
    {}

    ~TCPSession()
    {
        Destroy();
    }

    int GetSocket() const {return socket_fd;}
    uint32_t GetEPOLLEvents() const {return epoll_events;}

    RecvBuffer& GetRecvBuffer() {return recv_buffer;}
    SendBuffer& GetSendBuffer() {return send_buffer;}
};



#endif