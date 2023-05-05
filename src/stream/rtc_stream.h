#ifndef __RTC_STREAM_H_
#define __RTC_STREAM_H_
#include<rtc_base/rtc_certificate.h>
#include"base/event_loop.h"
#include<string>
#include<memory>
#include "pc/peer_connection.h"
#include"ice/port_allocator.h"
namespace grtc
{
    /**
     * 实时音视频流的基类，负责单
     * 个流的数据收发和质量控制
     * 内部成员 peerconnection 维持
     * 一个 webrtc p2p 通信连接
    */
    class RtcStream{
    public:
        RtcStream(EventLoop* el,PortAllocator* allocator ,uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id);
        virtual ~RtcStream();
        int start(rtc::RTCCertificate* certificate);
        virtual std::string create_offer() = 0;
    protected:
        EventLoop* el;
        uint64_t uid;
        std::string stream_name;
        bool audio;
        bool video;
        uint32_t log_id;
        std::unique_ptr<PeerConnection> pc;
        PortAllocator* _allocator;
    };
} // namespace grtc

#endif //__RTC_STREAM_H_
