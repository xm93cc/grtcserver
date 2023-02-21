#ifndef __BASE_SOCKET_H_
#define __BASE_SOCKET_H_
#include<sys/socket.h>

namespace grtc{
    int create_tcp_server(const char* host,int port);
    int tcp_accept(int sock, char* host , int* port);
    //设置sock为非阻塞
    int sock_setnonblock(int sock);
    //设置sock nodelay 属性
    int sock_setnodelay(int sock);
    //读取对端的host和port信息，返回-1为异常情况
    int sock_peer_2_str(int fd,char* host,int* port);
    //从fd（sock）中读取指定长度的字节数据，返回实际读取到字节数，-1为异常情况
    int sock_read_data(int sock,char* buf,size_t len);
    //向连接中写入数据
    int sock_write_data(int sock,const char* buf,size_t len);

}//namespace grtc

#endif //__BASE_SOCKET_H_