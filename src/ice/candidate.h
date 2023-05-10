/**
 * create by XM93CC
*/

/**
 * 候选者也叫 candidate，它包含一个网络地址信息，有主机候选者（host candidate），服务器反射候选者（srflx candidate），中继候选者（relay candidate）等，各种候选者代表的含义如下。
 *
 * 主机候选者（Local Address）是本地使用的 IP 地址和端口，例如通过 ifconfig/ipconfig 查看到 WLAN 网卡的 IP 地址是 192.168.0.105，并且准备使用 51417 端口。
 *
 * 服务器反射候选者（Server Reflexive Address）是 NAT 映射后使用的 IP 地址和端口。
 *
 * 中继候选者（Relayed Address） 是 TURN 服务器开辟的 IP 地址和端口，TURN 服务器是用来应对当通信对端 NAT 穿越失败无法建立 P2P 通信时，
 * 通过 TURN 服务器转发数据包，TURN 服务器一般都部署在带公网 IP 的机器上。
 *  Candidate 分类
        Host candidate：直接从本地网络接口获取，比如以太网、WiFi，或者一些虚拟
网络比如 VPN
    Server Reflexive candidate：由于 IPv4 地址是有限的，很多主机是没有公网
ip 的，它们访问公网是通过 NAT 映射的方式，这样它就会存在两个地址，一个是
本地的局域网地址，一个 NAT 映射的另一侧公网地址，正常情况下，主机是不知
道这个公网地址的，但是通过 STUN 或者 TURN 服务，是能够获取到这个公网地址，
这个公网地址就称之为 Server Reflexive candidate
    Relay candidate：一台独立的 TURN 服务器，可以为某个主机分配一个通信地址，
该主机可以通过这个地址与其它主机进行通信，TURN 服务负责在两个主机之间转
发数据，那么 TURN 服务分配的地址称之为 relay candidate
     Peer reflexive candidate：主机在做连通性检查的时候，会发送 STUN request
请求，如果对方能够成功返回 STUN response，response 会携带 NAT 映射的公网
侧地址，这个地址称为 Peer Reflexive candidate。
*/
#ifndef _ICE_CANDIDATE_H_
#define _ICE_CANDIDATE_H_
#include <string>
#include <rtc_base/logging.h>
#include "ice/ice_def.h"
#include <rtc_base/socket_address.h>

namespace grtc
{
class Candidate{
public:
    uint32_t get_priority(uint32_t type_preference, int network_adapter_preference, int relay_preference);
    std::string to_string() const;
public:
    IceCandidateComponent component;
    std::string protocol;
    rtc::SocketAddress address;
    int port = 0;
    uint32_t priority;
    std::string username;
    std::string password;
    std::string type;
    std::string foundation;   
};
} // namespace grtc


#endif //_ICE_CANDIDATE_H_