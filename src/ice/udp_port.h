/**
 * create by XM93CC
*/

#ifndef _UDP_PORT_H_
#define _UDP_PORT_H_
#include <string>
#include <rtc_base/socket_address.h>
#include "base/event_loop.h"
#include "base/network.h"
#include "base/async_upd_socket.h"
#include "ice/ice_def.h"
#include "ice/ice_credentials.h"
#include "ice/candidate.h"
#include <vector>
namespace grtc
{
class UDPPort : public sigslot::has_slots<>{
public:
    UDPPort(EventLoop* el,const std::string& transport_name,
    IceCandidateComponent component,IceParameters ice_params);
    int create_ice_candidate(Network* network, int min_port, int max_port, Candidate& c);
    ~UDPPort();
private:
    void _on_read_packet(AsyncUdpSocket* socket, char* buf, size_t size, const rtc::SocketAddress& addr, int64_t ts);
private:
    EventLoop* _el;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParameters _ice_params;
    int _socket = -1;
    std::unique_ptr<AsyncUdpSocket> _async_socket;
    rtc::SocketAddress _local_addr;
    std::vector<Candidate> _candidates;
};
} // namespace grtc

#endif