/**
 * transport controller header file
*/

#ifndef _TRANSPORT_CONTROLLER_H_
#define _TRANSPORT_CONTROLLER_H_
#include "base/event_loop.h"
#include "ice/ice_agent.h"
namespace grtc
{
class TransportController{
public:
    TransportController(EventLoop* el,PortAllocator* allocator);
    ~TransportController();
    int set_local_description(SessionDescription* desc);
private:
    EventLoop* _el;
    IceAgent* _ice_agent;
    
};
} // namespace grtc

#endif //_TRANSPORT_CONTROLLER_H_