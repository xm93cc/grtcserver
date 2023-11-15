#ifndef __ICE_CONNECTION__
#define __ICE_CONNECTION__
#include "base/event_loop.h"
#include "ice/candidate.h"
#include "ice/stun.h"
namespace grtc
{
class UDPPort;
class IceConnection{
public:
    IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate);
    ~IceConnection();
    const Candidate& remote_candidate() const {return _remote_candidate;} 
    void on_read_packet(const char* buf, size_t size, uint64_t ts);
    void handle_stun_binding_request(StunMessage* stun_msg);
    void send_stun_binding_response(StunMessage* stun_msg);
private:
    EventLoop* _el;
    UDPPort* _port;
    Candidate _remote_candidate;
};
} // namespace grtc

#endif
