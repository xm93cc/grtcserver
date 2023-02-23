#ifndef __PEER_CONNECTION_H_
#define __PEER_CONNECTION_H_
#include"base/event_loop.h"
#include"pc/session_description.h"
#include<string>
#include<memory>
namespace grtc
{
    class PeerConnection
    {
    private:
        EventLoop* _el;
        std::unique_ptr<SessionDescription> _local_desc;   
    public:
        PeerConnection(EventLoop* el);
        ~PeerConnection();
        //创建offer
        std::string create_offer();
    };
    
  
    
} // namespace grtc


#endif //__PEER_CONNECTION_H_