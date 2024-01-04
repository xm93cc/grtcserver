#include <rtc_base/helpers.h>
#include <rtc_base/time_utils.h>
#include "ice/udp_port.h"
#include "ice/ice_connection.h"

namespace grtc
{

const int RTT_RATIO = 3;

void ConnectionRequest::prepare(StunMessage* msg){
  msg->set_type(STUN_BINDING_REQUEST);
  std::string username;
  _connection->port()->create_stun_username(
      _connection->remote_candidate().username, &username);
  msg->add_attribute(
      std::make_unique<StunByteStringAttribute>(STUN_ATTR_USERNAME, username));
  msg->add_attribute(
      std::make_unique<StunUint64Attribute>(STUN_ATTR_ICE_CONTROLLING, 0));
  msg->add_attribute(
      std::make_unique<StunByteStringAttribute>(STUN_ATTR_USE_CANDIDATE, 0));
  // priority
  int type_pref = ICE_TYPE_PREFERENCE_PRFLX;
  uint32_t prflx_priority = (type_pref << 24) | (_connection->local_candidate().priority & 0x00FFFFFF);
  msg->add_attribute(std::make_unique<StunUint32Attribute>(STUN_ATTR_PRIORITY,prflx_priority));
  msg->add_message_integrity(_connection->remote_candidate().password);
  msg->add_fingerprint();
}

void ConnectionRequest::on_request_response(StunMessage* msg) {
  _connection->on_connection_request_response(this, msg);
}
void ConnectionRequest::on_request_error_response(StunMessage* msg) {
  _connection->on_connection_request_error_response(this, msg);
}

ConnectionRequest::ConnectionRequest(IceConnection* conn)
    : StunRequest(new StunMessage), _connection(conn) {}



const Candidate& IceConnection::local_candidate() const{
  return _port->candidates()[0];
}

void IceConnection::print_pings_since_last_response(std::string& pings,
                                                    int max) {
  std::stringstream ss;
  if (_pings_since_last_response.size() > max) {
    for (size_t i = 0; i < max; i++) {
      ss << rtc::hex_encode(_pings_since_last_response[i].id) << " ";
    }
    ss << "... " << _pings_since_last_response.size() - max << " more";
  } else {
    for (auto ping : _pings_since_last_response) {
      ss << rtc::hex_encode(ping.id) << " ";
    }
  }

  pings = ss.str();
}

void IceConnection::fail_and_destroy() {

}

void IceConnection::on_connection_request_error_response(
    ConnectionRequest* request, StunMessage* msg) {
  int rtt = request->elapsed();
  int error_code = msg->get_error_code_value();
  RTC_LOG(LS_WARNING) << to_string()
                      << ": Received: " << stun_method_to_string(msg->type())
                      << ", id=" << rtc::hex_encode(msg->transaction_id())
                      << ", rtt=" << rtt << ", code=" << error_code;
  if (STUN_ERROR_UNAUTHORIZED == error_code || STUN_ERROR_UNKNOWN_ATTRIBUTE == error_code || STUN_ERROR_SERVER_ERROR == error_code){
    // retry maybe recover
  }else {
    //失败 无法通过重试请求得以解决 ->  销毁IceConnection
    fail_and_destroy();
  }
}

int64_t IceConnection::last_received() {
  return std::max(std::max(_last_ping_received, _last_ping_response_received),
                  _last_data_received);
}

int IceConnection::receiving_timeout() {
  return WEAK_CONNECTION_RECEIVE_TIMEOUT;
}

void IceConnection::update_receiving(int64_t now_ts) {
  bool receiving;
  if (_last_ping_sent < _last_ping_response_received) {
    receiving = true;
  } else {
    receiving =
        last_received() > 0 && (now_ts < last_received() + receiving_timeout());
  }

  if (_receiving == receiving) {
    return;
  }

  RTC_LOG(LS_INFO) << to_string() << ": set receiving to " << receiving;
  _receiving = receiving;
  singal_state_change(this);
}

void IceConnection::set_write_state(WriteState state) {
  WriteState old_state = _write_state;
  _write_state = state;
  if (old_state != state) {
    RTC_LOG(LS_INFO) << to_string() << ": set write state from " << old_state
                     << " to " << state;
    singal_state_change(this);
  }
}

void IceConnection::received_ping_response(int rtt) {
  // old_rtt : new_rtt = 3 : 1
  // 5 10 20
  // rtt = 5
  // rtt = 5 * 0.75 + 10 * 0.25 = 3.75 + 2.5 = 6.25
  if (_rtt_samples > 0){
    _rtt = rtc::GetNextMovingAverage(_rtt, rtt, RTT_RATIO);
  }else{
    _rtt = rtt;
  }
  _last_ping_response_received = rtc::TimeMillis();
  //一旦收到成功的ping响应之后清理缓存
  _pings_since_last_response.clear();
  update_receiving(_last_ping_response_received);
  set_write_state(STATE_WRITABLE);
}

void IceConnection::on_connection_request_response(ConnectionRequest* request,
                                                   StunMessage* msg) {
  int rtt = request->elapsed();
  std::string pings;
  print_pings_since_last_response(pings, 5);
  RTC_LOG(LS_INFO) << to_string() << ": Received "
                      << stun_method_to_string(msg->type())
                      << ", id=" << rtc::hex_encode(msg->transaction_id())
                      << ", rtt=" << rtt << ", pings=" << pings;
  received_ping_response(rtt);
}

void IceConnection::_on_stun_send_packet(StunRequest* request, const char* data,
                                         size_t len) {
  int ret = _port->send_to(data, len, _remote_candidate.address);
  if (ret < 0) {
    RTC_LOG(LS_WARNING) << to_string()
                        << ": Failed to send STUN binding request: ret = "
                        << ret << ", id=" << rtc::hex_encode(request->id());
  }
}

IceConnection::IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate)
:_el(el),_port(port),_remote_candidate(remote_candidate)
{
  _requests.signal_send_packet.connect(this, &IceConnection::_on_stun_send_packet);
}

IceConnection::~IceConnection(){

}
void IceConnection::send_stun_binding_response(StunMessage* stun_msg){
    const StunByteStringAttribute* username_attr = stun_msg->get_byte_string(STUN_ATTR_USERNAME);
    if(!username_attr){
        RTC_LOG(LS_WARNING) << "send stun binding response error: no username";
        return;
    }
    StunMessage response;
    response.set_type(STUN_BINDING_RESPONSE);
    response.set_transaction_id(stun_msg->transaction_id());
    //4 + 8
    response.add_attribute(std::make_unique<StunXorAddressAttribute>(STUN_ATTR_XOR_MAPPED_ADDRESS, remote_candidate().address));
    //4 + 20
    response.add_message_integrity(_port->ice_pwd());
    //4 + 4
    response.add_fingerprint();
    send_response_message(response);
}

void IceConnection::send_response_message(const StunMessage& response) {
  const rtc::SocketAddress& addr = _remote_candidate.address;
  rtc::ByteBufferWriter buf;
  if (!response.write(&buf)) {
    return;
  }
  if (_port->send_to(buf.Data(), buf.Length(), addr) < 0) {
    RTC_LOG(LS_WARNING) << to_string() << ": send "
                        << stun_method_to_string(response.type())
                        << " error, to=" << addr.ToString() << ", id="
                        << rtc::hex_encode(response.transaction_id());
    return;
  }
  RTC_LOG(LS_INFO) << to_string() << ": sent "
                      << stun_method_to_string(response.type())
                      << " to=" << addr.ToString()
                      << ", id=" << rtc::hex_encode(response.transaction_id());
}

std::string IceConnection::to_string() {
  std::stringstream ss;
  ss << "Conn[" << this << ":" << _port->transport_name() << ":"
     << _port->component() << ":" << _port->local_addr().ToString() << "->"
     << _remote_candidate.address.ToString();
  return ss.str();
}

// rfc5245
// g : controlling candidate priority
// d : contrlled candidate priority
// conn priority = 2^32 * min(g, d) + 2 * max(g, d) + (g > d ? 1 : 0)
uint64_t IceConnection::priority() {
  uint32_t g = local_candidate().priority;
  uint32_t d = remote_candidate().priority;
  uint64_t priority = std::min(g, d);
  priority = priority << 32;
  return priority + 2 * std::max(g, d) + (g > d ? 1 : 0);
}

void IceConnection::handle_stun_binding_request(StunMessage* stun_msg){
    //role 的冲突问题 控制角色和受控角色
    //发送binding response
    send_stun_binding_response(stun_msg);
}

void IceConnection::on_read_packet(const char* buf, size_t len, uint64_t ts) {
  std::unique_ptr<StunMessage> stun_msg;
  std::string remote_ufrag;
  const Candidate& remote = _remote_candidate;
  if (!_port->get_stun_message(buf, len, remote.address, &stun_msg,
                               &remote_ufrag)) {
    //不是stun包 可能是dtls和rtp包
  } else if (!stun_msg) {
  } else {
    switch (stun_msg->type()) {
      case STUN_BINDING_REQUEST:
        if (remote_ufrag != remote.username) {
          RTC_LOG(LS_WARNING)
              << to_string() << ": Received "
              << stun_method_to_string(stun_msg->type())
              << " with bad username=" << remote_ufrag
              << " id=" << rtc::hex_encode(stun_msg->transaction_id());
          _port->send_binding_error_response(stun_msg.get(), remote.address,
                                             STUN_ERROR_UNAUTHORIZED,
                                             STUN_ERROR_REASON_UNAUTHORIZED);
        } else {
          RTC_LOG(LS_INFO) << to_string() << ": Received "
                           << stun_method_to_string(stun_msg->type()) << ", id="
                           << rtc::hex_encode(stun_msg->transaction_id());
          handle_stun_binding_request(stun_msg.get());
        }
        break;
      case STUN_BINDING_RESPONSE:
      case STUN_BINDING_ERROR_RESPONSE:
        stun_msg->validate_message_integrity(_remote_candidate.password);
        if (stun_msg->integrity_ok()) {
          _requests.check_response(stun_msg.get());
        }
        break;

      default:
        break;
    }
  }
}

void IceConnection::maybe_set_remote_ice_params(const IceParameters& params) {
  if (_remote_candidate.username == params.ice_ufrag &&
      _remote_candidate.password.empty()) {
    _remote_candidate.password = params.ice_pwd;
  }
}

bool IceConnection::stable(int64_t now) const {
  //todo
  return false;
}

void IceConnection::ping(int64_t now_ms) {
  ConnectionRequest* request = new ConnectionRequest(this);
  _pings_since_last_response.push_back(SentPing(request->id(), now_ms));
  RTC_LOG(LS_INFO) << to_string() << ": Sending STUN ping, id="
                   << rtc::hex_encode(request->id());
  _requests.send(request);
  _num_pings_sent++;
}

} // namespace grtc
