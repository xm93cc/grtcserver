#ifndef __RTC_STREAM_H_
#define __RTC_STREAM_H_
#include"base/event_loop.h"
#include<string>
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
        RtcStream(EventLoop* el,uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id, std::string& offer);
        virtual ~RtcStream();
    protected:
        EventLoop* el;
        uint64_t uid;
        std::string stream_name;
        bool audio;
        bool video;
        uint32_t log_id;
        std::string offer;
    };
} // namespace grtc

#endif //__RTC_STREAM_H_
