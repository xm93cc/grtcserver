#ifndef __TCP_CONNECTION_H_
#define __TCP_CONNECTION_H_
#include"base/event_loop.h"
namespace grtc
{
    class TcpConnection
    {
    public:
        /* data */
        int fd;
        char ip[64];
        int port;
        IOWatcher* io_watcher;
        EventLoop* _el;        

    public:
        TcpConnection(int fd);
        ~TcpConnection();
    };

} // namespace grtc


#endif //__TCP_CONNECTION_H_