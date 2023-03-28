/**
 * ice transport channel impl
*/


#include "ice/ice_transport_channel.h"

namespace grtc
{
IceTransportChannel::IceTransportChannel(EventLoop* el, const std::string& transport_name,
                         IceCandidateComponent component) :
                    _el(el),
                    _transport_name(transport_name),
                    _component(component)
{
}
//free 
IceTransportChannel::~IceTransportChannel(){}
} // namespace grtc
