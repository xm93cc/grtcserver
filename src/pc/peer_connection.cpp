//impl peer_connection.h
#include"pc/peer_connection.h"
namespace grtc
{
    PeerConnection::PeerConnection(EventLoop* el):_el(el)
    {
    }
    
    PeerConnection::~PeerConnection()
    {

    }

    std::string PeerConnection::create_offer(){
        _local_desc = std::make_unique<SessionDescription>(SdpType::k_offer);
        return _local_desc->to_string();
    }
} // namespace grtc