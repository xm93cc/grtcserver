/**
 * impl  transport controller header file
*/

#include <rtc_base/logging.h>
#include "pc/session_description.h"
#include "pc/transport_controller.h"
#include "pc/dtls_transport.h"

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
        
        DtlsTransport* dtls = new DtlsTransport(_ice_agent->get_channel(mid, IceCandidateComponent::RTP));
        dtls->set_local_certificate(_certificate);
        _add_dtls_transport(dtls);
    }
    _ice_agent->gathering_candidate();
    return 0;
}

int TransportController::set_remote_description(SessionDescription *sd) {
    if(!sd)
        return -1;

    for (auto content : sd->contents()){
        std::string mid = content->mid();
        if(sd->is_bundle(mid) && mid != sd->get_first_bundle_mid()){
            continue;
        }
        auto td = sd->get_transport_info(mid);

        _ice_agent->set_remote_ice_params(content->mid(), IceCandidateComponent::RTP, IceParameters(td->ice_ufrag, td->ice_pwd));
    }
    return 0;
    
}

void TransportController::_add_dtls_transport(DtlsTransport* dtls_transport) {
  auto iter = _dtls_transport_by_name.find(dtls_transport->transport_name());
  if (iter != _dtls_transport_by_name.end()) {
    delete iter->second;
  }
  _dtls_transport_by_name[dtls_transport->transport_name()] = dtls_transport;
}

 void TransportController::set_local_certificate(rtc::RTCCertificate* cert){
    _certificate = cert;
 }
} // namespace grtc
