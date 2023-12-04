#include "ice/udp_port.h"
#include "ice/ice_connection.h"

namespace grtc
{

IceConnection::IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate)
:_el(el),_port(port),_remote_candidate(remote_candidate)
{
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
        /* code */
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

} // namespace grtc
