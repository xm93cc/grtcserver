/*
 * @Author: XM93CC
 * @Date: 2023-05-31 10:42:10
 * @Description: 异步读取udp数据
 */

#ifndef __ASYNC_UDP_SOCKET_H_
#define __ASYNC_UDP_SOCKET_H_
#include <list>
#include <rtc_base/third_party/sigslot/sigslot.h>
#include <rtc_base/socket_address.h>
#include "base/event_loop.h"
namespace grtc
{
class UdpPacketData {
 private:
  char* _data;
  size_t _size;
  rtc::SocketAddress _addr;

 public:
  UdpPacketData(const char* data, size_t size, const rtc::SocketAddress& addr)
      : _data(new char[size]), _size(size), _addr(addr) 
  {
    memcpy(_data, data, size);
  }
  ~UdpPacketData() {
    if (_data) {
      delete[] _data;
      _data = nullptr;
    }
  }
};

class AsyncUdpSocket{
public:
    AsyncUdpSocket(EventLoop* el, int socket);
    ~AsyncUdpSocket();
    void recv_data();
    void send_data();
    int send_to(const char* buf, size_t len, const rtc::SocketAddress& addr);

    

    sigslot::signal5<AsyncUdpSocket*, char*, size_t, const rtc::SocketAddress&, int64_t> signal_read_packet;
private:
  int _add_udp_packet(const char* buf, size_t len, const rtc::SocketAddress& addr);

private:
    EventLoop* _el;
    int _socket;
    IOWatcher* _socket_watcher;
    char* _buf;
    size_t _size;
    
    std::list<UdpPacketData*> _udp_packet_list;
};

} // namespace grtc

#endif //__ASYNC_UDP_SOCKET_H_