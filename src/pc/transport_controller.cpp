/**
 * impl  transport controller header file
*/

#include <rtc_base/logging.h>
#include "pc/session_description.h"
#include "pc/transport_controller.h"
namespace grtc
{
    
TransportController::TransportController(EventLoop* el): 
        _el(el),_ice_agent(new IceAgent(el))
{

}
TransportController::~TransportController()
{
    
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
    }
    _ice_agent->gathering_candidate();
    return 0;
}
} // namespace grtc
