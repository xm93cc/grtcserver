// impl tcp_connection.h
#include"server/tcp_connection.h"
namespace grtc
{

    TcpConnection::TcpConnection(int fd) : fd(fd),querybuf(sdsempty()){}
    TcpConnection::~TcpConnection(){
        sdsfree(querybuf);
    }
    
} // namespace grtc
