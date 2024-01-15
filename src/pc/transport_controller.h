/**
 * transport controller header file
*/

#ifndef _TRANSPORT_CONTROLLER_H_
#define _TRANSPORT_CONTROLLER_H_
#include <map>
#include "base/event_loop.h"
#include "ice/ice_agent.h"
namespace grtc
{

class DtlsTransport;

class TransportController : public sigslot::has_slots<>{
public:
    TransportController(EventLoop* el,PortAllocator* allocator);

    ~TransportController();

    int set_local_description(SessionDescription* desc);

    int set_remote_description(SessionDescription* sd);

    sigslot::signal4<TransportController*, const std::string&, IceCandidateComponent, const std::vector<Candidate>&> signal_candidate_allocate_done;

private:
    void on_candidate_allocate_done(IceAgent* agent, const std::string& transport_name,
                                    IceCandidateComponent component, const std::vector<Candidate>& candidates);
    
    void  _add_dtls_transport(DtlsTransport* dtls_transport);

private:
    EventLoop* _el;
    IceAgent* _ice_agent;
    std::map<std::string, DtlsTransport*> _dtls_transport_by_name;
    
};
} // namespace grtc

#endif //_TRANSPORT_CONTROLLER_H_