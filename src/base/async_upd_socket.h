/*
 * @Author: XM93CC
 * @Date: 2023-05-31 10:42:10
 * @Description: 异步读取udp数据
 */

#ifndef __ASYNC_UDP_SOCKET_H_
#define __ASYNC_UDP_SOCKET_H_
#include <rtc_base/third_party/sigslot/sigslot.h>
#include <rtc_base/socket_address.h>
#include "base/event_loop.h"
namespace grtc
{
class AsyncUdpSocket{
public:
    AsyncUdpSocket(EventLoop* el, int socket);
    ~AsyncUdpSocket();
    void recv_date();

    sigslot::signal5<AsyncUdpSocket*, char*, size_t, const rtc::SocketAddress&, int64_t> signal_read_packet;
private:
    EventLoop* _el;
    int _socket;
    IOWatcher* _socket_watcher;
    char* _buf;
    size_t _size;
     
};

} // namespace grtc

#endif //__ASYNC_UDP_SOCKET_H_