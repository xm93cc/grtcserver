#ifndef __BASE_SOCKET_H_
#define __BASE_SOCKET_H_
#include<sys/socket.h>

namespace grtc{
    int create_tcp_server(const char* host,int port);
    int tcp_accept(int sock, char* host , int* port);

}//namespace grtc

#endif //__BASE_SOCKET_H_