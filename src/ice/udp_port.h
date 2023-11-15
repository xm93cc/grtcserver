/**
 * create by XM93CC
*/

#ifndef _UDP_PORT_H_
#define _UDP_PORT_H_
#include <string>
#include <map>
#include <rtc_base/socket_address.h>
#include "base/event_loop.h"
#include "base/network.h"
#include "base/async_upd_socket.h"
#include "ice/ice_def.h"
#include "ice/ice_credentials.h"
#include "ice/candidate.h"
#include <vector>
#include "ice/stun.h"
namespace grtc
{

class IceConnection;
typedef std::map<rtc::SocketAddress, IceConnection*> AddressMap;
class UDPPort : public sigslot::has_slots<>{
public:
    UDPPort(EventLoop* el,const std::string& transport_name,
    IceCandidateComponent component,IceParameters ice_params);
    int create_ice_candidate(Network* network, int min_port, int max_port, Candidate& c);
    ~UDPPort();
    std::string ice_ufrag(){return _ice_params.ice_ufrag;}
    std::string ice_pwd(){return _ice_params.ice_pwd;}
    //bool get_stun_message(const char* data, size_t len, std::unique_ptr<StunMessage>* out_msg);
    // bool get_stun_message(const char* data, size_t len,
    //                         const rtc::SocketAddress& addr, std::unique_ptr<StunMessage>* out_msg,
    //                        std::string* out_username);
    std::string to_string();
    //发送异常的stun消息
    void send_binding_error_response(StunMessage * stun_msg, const rtc::SocketAddress & addr, int err_code, const std::string & reason);
    IceConnection* create_connection(const Candidate& candidate);

    IceConnection* get_connection(const rtc::SocketAddress& addr);
    //定义信号
    sigslot::signal4<UDPPort*, const rtc::SocketAddress&,StunMessage*,const std::string&> signal_unknown_address;
private:
    void _on_read_packet(AsyncUdpSocket* socket, char* buf, size_t size, const rtc::SocketAddress& addr, int64_t ts);
    bool get_stun_message(const char * data, size_t len, const rtc::SocketAddress & addr, std::unique_ptr<StunMessage>* out_msg, std::string * out_username);
    bool _parse_stun_username(StunMessage* stun_msg, std::string* local_ufrag, std::string* remote_ufrag);
private:
    EventLoop* _el;
    std::string _transport_name;
    IceCandidateComponent _component;
    IceParameters _ice_params;
    int _socket = -1;
    std::unique_ptr<AsyncUdpSocket> _async_socket;
    rtc::SocketAddress _local_addr;
    std::vector<Candidate> _candidates;
    AddressMap _connections;
};
} // namespace grtc

#endif