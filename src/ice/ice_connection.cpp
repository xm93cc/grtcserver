#include "ice/udp_port.h"
#include "ice/ice_connection.h"

namespace grtc
{

IceConnection::IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate)
:_el(el),_port(port),_remote_candidate(remote_candidate)
{
}

IceConnection::~IceConnection(){

}

} // namespace grtc
