/**
 * create by XM93CC
*/

#ifndef _UDP_PORT_H_
#define _UDP_PORT_H_
#include <string>
#include <rtc_base/socket_address.h>
#include "base/event_loop.h"
#include "base/network.h"
#include "ice/ice_def.h"
#include "ice/ice_credentials.h"
#include "ice/candidate.h"
#include <vector>
namespace grtc
{
class UDPPort{
public:
    UDPPort(EventLoop* el,const std::string& transport_name,
    IceCandidateComponent component,IceParameters ice_params);
    int create_ice_candidate(Network* network, int min_port, int max_port, Candidate& c);
    ~UDPPort();
private:
    EventLoop _el;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParameters _ice_params;
    int _socket = -1;
    rtc::SocketAddress _local_addr;
    std::vector<Candidate> _candidates;
};
} // namespace grtc

#endif