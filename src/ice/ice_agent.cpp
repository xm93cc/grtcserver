/**
 * impl ice agent 
*/


#include "ice/ice_agent.h"
#include <algorithm>
namespace grtc
{
    
IceAgent::IceAgent(EventLoop* el,PortAllocator* allocator):_el(el),_allocator(allocator){}
IceAgent::~IceAgent(){}
/**不存在即创建channel */
bool IceAgent::create_channel(EventLoop* el, const std::string& transport_name, IceCandidateComponent component)
{
    if(get_channel(transport_name, component)){
        return true;
    }
    auto channel = new IceTransportChannel(el, _allocator, transport_name, component);
    _channels.push_back(channel);
    return true;
}

IceTransportChannel* IceAgent::get_channel(const std::string& transport_name, IceCandidateComponent component)
{  
    auto iter = _get_channel(transport_name,component);
    return iter == _channels.end() ? nullptr : *iter;
}

std::vector<IceTransportChannel *>::iterator IceAgent::_get_channel(const std::string transport_name,
                                                                    IceCandidateComponent component)
{
    return std::find_if(_channels.begin(), _channels.end(), [transport_name,component](IceTransportChannel* channel){
        return transport_name == channel->transport_name() && component == channel->component();
    });
}


void IceAgent::gathering_candidate(){
    for(auto channel : _channels){
        channel->gathering_candidate();
    }
}
} // namespace grtc
