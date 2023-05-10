/**
 * ice transport channel
*/

#ifndef __ICE_TRANSPORT_CHANNEL_H_
#define __ICE_TRANSPORT_CHANNEL_H_
#include <string> 
#include <vector>
#include <rtc_base/third_party/sigslot/sigslot.h>
#include "base/event_loop.h"
#include "ice/ice_def.h"
#include "ice/port_allocator.h"
#include "ice/ice_credentials.h"
#include "ice/candidate.h"
namespace grtc
{
class IceTransportChannel{
public:
    IceTransportChannel(EventLoop* el,PortAllocator* allocator, const std::string& transport_name,
                         IceCandidateComponent component);
    virtual ~IceTransportChannel();
    std::string transport_name() {return _transport_name;}
    IceCandidateComponent component() {return _component;}
    void gathering_candidate();
    void set_ice_params(const IceParameters& ice_params);
    sigslot::signal2<IceTransportChannel*, const std::vector<Candidate>&> signal_candidate_allocate_done;
    
private:
    EventLoop* _el;
    PortAllocator* _allocator;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParameters _ice_params;
    std::vector<Candidate> _local_candidates;
};
    
} // namespace grtc

#endif //__ICE_TRANSPORT_CHANNEL_H_