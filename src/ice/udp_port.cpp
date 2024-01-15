/**
 * create by XM93CC
 */
#include <sstream>
#include "base/socket.h"
#include "ice/udp_port.h"
#include <rtc_base/crc32.h>
#include <rtc_base/string_encode.h>
#include <rtc_base/logging.h>
#include <ice/stun.h>
#include "ice/ice_connection.h"
#include "ice/udp_port.h"
namespace grtc
{
UDPPort::UDPPort(EventLoop *el, const std::string& transport_name,
                IceCandidateComponent component, IceParameters ice_params):
                _el(el),
                _transport_name(transport_name),
                _component(component),
                _ice_params(ice_params)
{

}

std::string compute_foundation(const std::string& type, const std::string& protocol,
                               const std::string& relay_protocol, const rtc::SocketAddress& base)
{
    std::stringstream ss;
    ss << type << base.HostAsURIString() << protocol << relay_protocol;
    return std::to_string(rtc::ComputeCrc32(ss.str()));
}   

bool UDPPort::get_stun_message(const char* data, size_t len,
                             const rtc::SocketAddress& addr, std::unique_ptr<StunMessage>* out_msg,
                             std::string* out_username)
{
    if(!StunMessage::validate_fingerprint(data, len)){
        return false;
    }
    std::unique_ptr<StunMessage> stun_msg = std::make_unique<StunMessage>();
    rtc::ByteBufferReader buf(data, len);
    if(!stun_msg->read(&buf)){
        return false;
    }
    if(STUN_BINDING_REQUEST == stun_msg->type()){
        if(!stun_msg->get_byte_string(STUN_ATTR_USERNAME)||!stun_msg->get_byte_string(STUN_ATTR_MESSAGE_INTEGRITY)){
            //todo send error request
            RTC_LOG(LS_WARNING) << to_string() << ": recevied "
            << stun_method_to_string(stun_msg->type()) 
            << " without username/M-I from "
            << addr.ToString(); 
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_BAD_REQUEST, STUN_ERROR_REASON_BAD_REQUEST);
            return true;
        }
        std::string local_ufrag;
        std::string remote_ufrag;
        if(!_parse_stun_username(stun_msg.get(), &local_ufrag, &remote_ufrag) || local_ufrag != _ice_params.ice_ufrag){
            RTC_LOG(LS_WARNING) << to_string() << ": recevied "
            << stun_method_to_string(stun_msg->type()) 
            << " with bad local_ufarg: "<< _local_addr.ToString() << " from "
            << addr.ToString(); 
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_UNAUTHORIZED, STUN_ERROR_REASON_UNAUTHORIZED);
            return true;
        }
        if(stun_msg->validate_message_integrity(_ice_params.ice_pwd) != StunMessage::IntegrityStatus::k_integrity_ok){
            RTC_LOG(LS_WARNING) << to_string() << ": recevied "
            << stun_method_to_string(stun_msg->type()) 
            << " with Bad M-I from "
            << addr.ToString(); 
            send_binding_error_response(stun_msg.get(), addr, STUN_ERROR_UNAUTHORIZED, STUN_ERROR_REASON_UNAUTHORIZED);
            return true;
        }
        *out_username = remote_ufrag;
    }
    
    *out_msg = std::move(stun_msg);
    return true;
}

bool UDPPort::_parse_stun_username(StunMessage* stun_msg, std::string* local_ufrag, std::string* remote_ufrag){
    local_ufrag->clear();
    remote_ufrag->clear();

   const StunByteStringAttribute* attr = stun_msg->get_byte_string(STUN_ATTR_USERNAME);
   if(!attr){
    return false;
   }
   // RFRAG:LFRAG
   std::string username = attr->get_string();
   std::vector<std::string> fileds;
   rtc::split(username, ':', &fileds);
   if (fileds.size() != 2) {
    return false;
   }
    *local_ufrag = fileds[0];
    *remote_ufrag = fileds[1];

    //RTC_LOG(LS_WARNING) << "local_ufrag: " << *local_ufrag << "  remote_ufrag: " << *remote_ufrag;
    return true;

}

IceConnection* UDPPort::create_connection(const Candidate& remote_candidate){
    IceConnection* conn = new IceConnection(_el, this, remote_candidate);
    auto ret = _connections.insert(std::make_pair(conn->remote_candidate().address,conn));
    if(ret.second == false && ret.first->second != conn){
        RTC_LOG(LS_WARNING) << to_string() << ": create ice connection on "
        << "an existing remote address, addr: " << conn->remote_candidate().address.ToString();
        ret.first->second = conn;
        //todo 处理重复创建连接的情况
    }
    return conn;
}

IceConnection* UDPPort::get_connection(const rtc::SocketAddress& addr){
    auto iter = _connections.find(addr);
    return iter == _connections.end() ? nullptr : iter->second;
}

void UDPPort::_on_read_packet(AsyncUdpSocket* /*socket*/, char* buf, size_t size, const rtc::SocketAddress& addr, int64_t ts){
    //处理重复创建IceConnection问题（连通性检查会每隔48ms发一次请求）
    if(IceConnection* conn = get_connection(addr)){
        conn->on_read_packet(buf, size, ts);
        return;
    }
    std::unique_ptr<StunMessage> stun_msg;
    std::string remote_ufrag;
    bool ret = get_stun_message(buf, size, addr, &stun_msg, &remote_ufrag);
    //RTC_LOG(LS_WARNING) << "========ret: " << ret;

    if(!ret || !stun_msg){
        return;
    }

    if(STUN_BINDING_REQUEST == stun_msg->type()){
        RTC_LOG(LS_INFO) << to_string() << ": Received "
                         << stun_method_to_string(stun_msg->type()) 
                         << " id="  << rtc::hex_encode(stun_msg->transaction_id())
                         << " from " << addr.ToString();
        signal_unknown_address(this, addr, stun_msg.get(),remote_ufrag);
    }
}

int UDPPort::create_ice_candidate(Network* network, int min_port, int max_port, Candidate& c)
{
    _socket = create_udp_socket(network->ip().family());
    if (_socket < 0)
    {
        return -1;
    }
    if (sock_setnonblock(_socket) != 0){
        return -1;
    }
    sockaddr_in addr_in;
    addr_in.sin_family = network->ip().family();
    addr_in.sin_addr = network->ip().ipv4_address();
    if(sock_bind(_socket, (struct sockaddr*)&addr_in,sizeof(sockaddr) , min_port, max_port) != 0)
    {
        return -1;
    }
    int port = 0;
    if(sock_get_address(_socket, nullptr, &port) != 0){
        return -1;
    }
    _local_addr.SetIP(network->ip());
    _local_addr.SetPort(port);
    _async_socket = std::make_unique<AsyncUdpSocket>(_el, _socket);
    _async_socket->signal_read_packet.connect(this, &UDPPort::_on_read_packet);
    RTC_LOG(LS_INFO) << "prepared socket address: " << _local_addr.ToString();
    
    c.component = _component;
    c.protocol = "udp";
    c.address = _local_addr;
    c.port = port;
    c.priority = c.get_priority(ICE_TYPE_PREFERENCE_HOST, 0, 0);
    c.username = _ice_params.ice_ufrag;
    c.password = _ice_params.ice_pwd;
    c.type = LOCAL_PORT_TYPE;
    c.foundation = compute_foundation(c.type, c.protocol, "", c.address);;
    _candidates.push_back(c);
    return 0;
}

UDPPort::~UDPPort()
{

}


std::string UDPPort::to_string(){
    std::stringstream ss;
    ss << "Port[" << this << ":" << _transport_name << ":" << _component
    << ":" << _ice_params.ice_ufrag << ":" << _ice_params.ice_pwd
    << ":" << _local_addr.ToString() << "]" ;
    return ss.str();
}


int UDPPort::send_to(const char* buf, size_t len, const rtc::SocketAddress& addr){
    if(!_async_socket){
        return -1;
    }
    return _async_socket->send_to(buf, len, addr);
}

void UDPPort::send_binding_error_response(StunMessage* stun_msg,
                                          const rtc::SocketAddress& addr,
                                          int err_code,
                                          const std::string& reason) {
  if (!_async_socket) {
    return;
  }
  StunMessage response;
  response.set_type(STUN_BINDING_ERROR_RESPONSE);
  response.set_transaction_id(stun_msg->transaction_id());
  auto error_attr = StunAttribute::create_error_code();
  error_attr->set_code(err_code);
  error_attr->set_reason(reason);
  response.add_attribute(std::move(error_attr));
  if (err_code != STUN_ERROR_BAD_REQUEST &&
      err_code != STUN_ERROR_UNAUTHORIZED) {
    response.add_message_integrity(_ice_params.ice_pwd);
  }
  response.add_fingerprint();
  rtc::ByteBufferWriter buf;
  if (!response.write(&buf)) {
    return;
  }
  int ret = _async_socket->send_to(buf.Data(), buf.Length(), addr);
  if (ret < 0) {
    RTC_LOG(LS_WARNING) << to_string() << " send "
                        << stun_method_to_string(response.type())
                        << " error, ret=" << ret << ", to=" << addr.ToString();
  } else {
    RTC_LOG(LS_WARNING) << to_string() << " send "
                        << stun_method_to_string(response.type())
                        << ", reason=" << reason << ", to=" << addr.ToString();
  }
}


void UDPPort::create_stun_username(const std::string& remote_username, std::string* stun_attr_username){
    stun_attr_username->clear();
    *stun_attr_username = remote_username;
    stun_attr_username->append(":");
    stun_attr_username->append(_ice_params.ice_ufrag);
}
} // namespace grtc
