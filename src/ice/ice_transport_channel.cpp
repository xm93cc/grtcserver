/**
 * ice transport channel impl
*/
#include <rtc_base/time_utils.h>
#include <rtc_base/logging.h>
#include "ice/ice_transport_channel.h"
#include "ice/udp_port.h"
#include "ice/ice_connection.h"

namespace grtc
{

const int  PING_INTERVAL_DIFF = 5;

void ice_ping_cb(EventLoop* /*el*/, TimerWatcher* /*w*/, void* data){
     IceTransportChannel* channel = (IceTransportChannel*)data;
     channel->_on_check_and_ping();
}


IceTransportChannel::IceTransportChannel(EventLoop* el,PortAllocator* allocator, const std::string& transport_name,
                         IceCandidateComponent component) :
                    _el(el),
                    _allocator(allocator),
                    _transport_name(transport_name),
                    _component(component),
                    _ice_controller(new IceController(this))
{
     RTC_LOG(LS_INFO) << "ice transport channel created, transport_name: "<< _transport_name << ", component: " << _component;
     _ping_watcher = _el->create_timer(ice_ping_cb, this, true);
}
//free 
IceTransportChannel::~IceTransportChannel(){
     if(_ping_watcher){
          _el->delete_timer(_ping_watcher);
          _ping_watcher = nullptr;
     }
}

void IceTransportChannel::set_ice_params(const IceParameters& ice_params){
     RTC_LOG(LS_INFO) << "set ICE param transport_name: " << _transport_name
                         << ", component: " << _component << ", ufrag: "
                         << ice_params.ice_ufrag << ", pwd: " << ice_params.ice_pwd;
     _ice_params = ice_params;
}

void IceTransportChannel::gathering_candidate()
{
     if(_ice_params.ice_ufrag.empty() || _ice_params.ice_pwd.empty()){
          RTC_LOG(LS_WARNING) << "cannot gathering condidate becase ICE param is empty "
          << ", transport_name: " << _transport_name
          << ", component: " << _component
          << ", ufrag: " << _ice_params.ice_ufrag
          << ", pwd: " << _ice_params.ice_pwd;
          return;
     }
     auto network_list = _allocator->get_networks();
     if(network_list.empty()){
           RTC_LOG(LS_WARNING) << "cannot gathering condidate becase ICE network_list is empty "
          << ". transport_name: " << _transport_name
          << ", component: " << _component;
          return;
     }
     for(auto network : network_list){     
          UDPPort* port = new UDPPort(_el, _transport_name, _component, _ice_params);
          //处理连接检查传递的地址
          port->signal_unknown_address.connect(this,&IceTransportChannel::_on_unknown_address);
          Candidate c;
          int ret = port->create_ice_candidate(network, _allocator->get_min_port(), _allocator->get_max_port(), c);
          if (ret != 0)
          {
               continue;
          } 
          _local_candidates.push_back(c);
     }
     
     //将从此 递传到peer(PeerConnection) 层
     signal_candidate_allocate_done(this, _local_candidates);
}



void IceTransportChannel::set_remote_ice_params(const IceParameters& ice_params){
      RTC_LOG(LS_INFO) << "set remote ICE param transport_name: " << _transport_name
                         << ", component: " << _component << ", ufrag: "
                         << ice_params.ice_ufrag << ", pwd: " << ice_params.ice_pwd;
     _remote_ice_params = ice_params;
     for(auto conn : _ice_controller->connectios()){
          conn->maybe_set_remote_ice_params(ice_params);
     }
     _sort_connections_and_update_state();
}


std::string IceTransportChannel::to_string(){
     std::stringstream ss;
     ss << "Channel[" << this << ":" << _transport_name << ":" << _component << "]";
     return ss.str();
}

void  IceTransportChannel::_on_unknown_address(UDPPort* port, const rtc::SocketAddress& addr,StunMessage* msg,const std::string& remote_ufrag){
     const StunUint32Attribute* priority_attr = msg->get_uint32(STUN_ATTR_PRIORITY);
     if(!priority_attr){
          RTC_LOG(LS_WARNING) << to_string() << ": priority not found in the"
          << " binding request message, remote_addr: " << addr.ToString();
          //连通性检查priority 不存在向客户端返回一个错误的响应
          port->send_binding_error_response(msg, addr, STUN_ERROR_BAD_REQUEST,STUN_ERROR_REASON_BAD_REQUEST);
          return; 
     }
     uint32_t remote_priority = priority_attr->value();
     Candidate remote_candidate;
     remote_candidate.component = _component;
     remote_candidate.protocol = "udp";
     remote_candidate.address = addr;
     remote_candidate.username = remote_ufrag;
     remote_candidate.password = _remote_ice_params.ice_pwd;
     remote_candidate.priority = remote_priority;
     remote_candidate.type = PRFLX_PORT_TYPE;
     RTC_LOG(LS_INFO) << to_string() <<  " create peer reflexive candidate: " 
     << remote_candidate.to_string();
     IceConnection* conn = port->create_connection(remote_candidate);
     if(!conn){
          RTC_LOG(LS_WARNING) <<to_string() << "create connection from peer reflexive candidate error remote_addr: "
          << addr.ToString();
          port->send_binding_error_response(msg, addr, STUN_ERROR_SERVER_ERROR,STUN_ERROR_REASON_SERVER_ERROR);
          return;
     }
     RTC_LOG(LS_INFO) <<to_string() << "create connection from peer reflexive candidate success remote_addr: "
     << addr.ToString();
     _add_connection(conn);
     conn->handle_stun_binding_request(msg);
     _sort_connections_and_update_state();
}

void IceTransportChannel::_maybe_swtich_selected_connection(IceConnection* conn){
    if(!conn){
     return;
    }

    IceConnection* old_selected_connection = _selected_connection;
    if(old_selected_connection){
     old_selected_connection->set_selected(false);
     RTC_LOG(LS_INFO)<< conn->to_string() << ": Previous connection: "
                     << old_selected_connection->to_string();
    }
    RTC_LOG(LS_INFO) << to_string() << ": New selected connection: "
                     << conn->to_string();
    _selected_connection = conn;
    _selected_connection->set_selected(true);
    _ice_controller->set_selected_connection(_selected_connection);
}

void IceTransportChannel::_sort_connections_and_update_state(){
     _maybe_swtich_selected_connection(_ice_controller->sort_and_switch_connection());
     _maybe_start_pinging();
}

void IceTransportChannel::_maybe_start_pinging(){
     if(_start_pinging){
          return;
     }

     if(_ice_controller->has_pingable_connection()){
          RTC_LOG(LS_INFO) << to_string() << " : Have a pingable connection "
          << "for the first time, staring to ping";
          //定时器
          _el->start_timer(_ping_watcher, _cur_ping_interval * 1000);
          _start_pinging = true;
     }

}



void IceTransportChannel::_on_check_and_ping(){
  auto result = _ice_controller->selected_connection_to_ping(_last_ping_sent_ms - PING_INTERVAL_DIFF);
  RTC_LOG(LS_WARNING) << "=========>> conn: " << result.conn
                      << ", ping interval: " << result.ping_interval;
  if (result.conn){
     IceConnection* conn = (IceConnection*)result.conn;
     _ping_connection(conn);
     _ice_controller->mark_connection_pinged(conn);
  }
  if (_cur_ping_interval != result.ping_interval) {
    _cur_ping_interval = result.ping_interval;
    _el->stop_timer(_ping_watcher);
    _el->start_timer(_ping_watcher, _cur_ping_interval * 1000);
  }
}

void IceTransportChannel::_ping_connection(IceConnection* conn) {
  _last_ping_sent_ms = rtc::TimeMillis();
  conn->ping(_last_ping_sent_ms);
}

void IceTransportChannel::_on_connection_state_change(IceConnection* /*conn*/) {
  _sort_connections_and_update_state();
}

void IceTransportChannel::_add_connection(IceConnection* conn) {
  conn->singal_state_change.connect(
      this, &IceTransportChannel::_on_connection_state_change);
  _ice_controller->add_connection(conn);
}
} // namespace grtc
