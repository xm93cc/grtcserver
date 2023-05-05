/**
 * ice transport channel impl
*/

#include <rtc_base/logging.h>
#include "ice/ice_transport_channel.h"

namespace grtc
{
IceTransportChannel::IceTransportChannel(EventLoop* el,PortAllocator* allocator, const std::string& transport_name,
                         IceCandidateComponent component) :
                    _el(el),
                    _allocator(allocator),
                    _transport_name(transport_name),
                    _component(component)
{
     RTC_LOG(LS_INFO) << "ice transport channel created, transport_name: "<< _transport_name << ", component: " << _component;
}
//free 
IceTransportChannel::~IceTransportChannel(){}


void IceTransportChannel::gathering_candidate()
{
     
}
} // namespace grtc
