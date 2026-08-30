#ifndef SOCKETICP_H
#define SOCKETICP_H

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

class SocketICP
{
    int socket;
    sockaddr_in addr;  
};

#endif