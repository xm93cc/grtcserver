#ifndef __ICE_CONNECTION__
#define __ICE_CONNECTION__
#include "base/event_loop.h"
#include "ice/candidate.h"
namespace grtc
{
class UDPPort;
class IceConnection{
public:
    IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate);
    ~IceConnection();
    const Candidate& remote_candidate() const {return _remote_candidate;} 
private:
    EventLoop* _el;
    UDPPort* _port;
    Candidate _remote_candidate;
};
} // namespace grtc

#endif
