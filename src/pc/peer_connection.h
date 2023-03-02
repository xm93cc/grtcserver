#ifndef __PEER_CONNECTION_H_
#define __PEER_CONNECTION_H_
#include <rtc_base/rtc_certificate.h>
#include "base/event_loop.h"
#include "pc/session_description.h"
#include <string>
#include <memory>
namespace grtc
{

    struct RTCOfferAnswerOptions
    {
        bool recv_audio = true;
        bool recv_video = true;
        bool use_rtp_mux = true; // 是否使用bundle（音视频复用通道）
        bool send_audio = true;
        bool send_video = true;
        bool use_rtcp_mux = true;
        bool dtls_on = true;//开启安全传输 
    };
    class PeerConnection
    {
    private:
        EventLoop *_el;
        std::unique_ptr<SessionDescription> _local_desc;
        rtc::RTCCertificate* _certificate = nullptr;

    public:
        PeerConnection(EventLoop *el);
        ~PeerConnection();
        // 创建offer
        std::string create_offer(const RTCOfferAnswerOptions &options);
        //证书
        int init(rtc::RTCCertificate* certificate);
    };

} // namespace grtc

#endif //__PEER_CONNECTION_H_