/**
 * impl  transport controller header file
*/

#include <rtc_base/logging.h>
#include "pc/session_description.h"
#include "pc/transport_controller.h"
namespace grtc
{
    
TransportController::TransportController(EventLoop* el,PortAllocator* allocator): 
        _el(el),_ice_agent(new IceAgent(el,allocator))
{
    _ice_agent->signal_candidate_allocate_done.connect(this, &TransportController::on_candidate_allocate_done);
}
TransportController::~TransportController()
{
    
}

void TransportController::on_candidate_allocate_done(IceAgent* /*agent*/, const std::string& transport_name,
                                    IceCandidateComponent component, const std::vector<Candidate>& candidates)
{
    signal_candidate_allocate_done(this, transport_name, component, candidates); 
}

int TransportController::set_local_description(SessionDescription* desc)
{
    if(!desc){
        RTC_LOG(LS_WARNING) << "desc is null";
        return -1;
    }

    for(auto content : desc->contents()){
        std::string mid = content->mid();
        //只创建一次
        if(desc->is_bundle(mid) && mid != desc->get_first_bundle_mid()){
            continue;
        }
        _ice_agent->create_channel(_el, mid, IceCandidateComponent::RTP);
        auto td = desc->get_transport_info(mid);
        if (td)
        {
            _ice_agent->set_ice_params(mid,IceCandidateComponent::RTP,IceParameters(td->ice_ufrag,td->ice_pwd));
        }
        
        
    }
    _ice_agent->gathering_candidate();
    return 0;
}
} // namespace grtc
