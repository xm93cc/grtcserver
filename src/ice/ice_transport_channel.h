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
#include "ice/stun.h"
#include "ice/udp_port.h"
#include "ice/ice_controller.h"
namespace grtc
{
class IceTransportChannel : public sigslot::has_slots<> {
public: 
    IceTransportChannel(EventLoop* el,PortAllocator* allocator, const std::string& transport_name,
                         IceCandidateComponent component);
    
    virtual ~IceTransportChannel();
    
    std::string& transport_name() {return _transport_name;}
    
    IceCandidateComponent component() {return _component;}
    
    void gathering_candidate();
    
    void set_ice_params(const IceParameters& ice_params);
    
    void set_remote_ice_params(const IceParameters& ice_params);
    
    void _on_unknown_address(UDPPort* port, const rtc::SocketAddress& addr,StunMessage* msg,const std::string& remote_ufrag);
    
    std::string to_string();

//signal def
public:
    sigslot::signal1<IceTransportChannel*> signal_writable_state;
    sigslot::signal1<IceTransportChannel*> signal_receiving_state;
    sigslot::signal2<IceTransportChannel*, const std::vector<Candidate>&> signal_candidate_allocate_done;
    sigslot::signal4<IceTransportChannel*, const char*, size_t, int64_t> signal_read_packet;

private:
    void _sort_connections_and_update_state();
    
    void _maybe_start_pinging();
    
    void _add_connection(IceConnection* conn);
    
    void _on_check_and_ping();
    
    void _ping_connection(IceConnection* conn);

    friend void ice_ping_cb(EventLoop* /*el*/, TimerWatcher* /*w*/, void* data);

    void _on_connection_state_change(IceConnection* conn);

    void _maybe_switch_selected_connection(IceConnection* conn);

    void _switch_selected_connection(IceConnection* conn);

    void _on_connection_destroyed(IceConnection* conn);

    //连接探活
    void _update_connection_state();

    void _update_state();

    void _set_writable(bool writable);

    void _set_receiving(bool receiving);
    
    void _on_read_packet(IceConnection* conn, const char* buf, size_t len, int64_t ts);

private:
    EventLoop* _el;
    PortAllocator* _allocator;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParameters _ice_params;
    IceParameters _remote_ice_params;
    std::vector<Candidate> _local_candidates;
    std::unique_ptr<IceController> _ice_controller;
    bool _start_pinging = false;
    TimerWatcher* _ping_watcher = nullptr;
    int64_t _last_ping_sent_ms = 0;
    int64_t _cur_ping_interval = WEAK_PING_INTERVAL;
    IceConnection* _selected_connection = nullptr;
    bool _writable = false;
    bool _receiving = false;
};
    
} // namespace grtc

#endif //__ICE_TRANSPORT_CHANNEL_H_