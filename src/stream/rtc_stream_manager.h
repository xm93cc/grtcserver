#ifndef __RTC_STREAM_MANAGER_H_
#define __RTC_STREAM_MANAGER_H_
#include<string>
#include"base/event_loop.h"
#include<unordered_map>
namespace grtc
{
    class PushStream;
    /**
     * 流管理器，
     * 管理所有的推流和拉流会话
     * 控制流之间的数据转发
     * 
     */
    class RtcStreamManager{
     public:
        RtcStreamManager(EventLoop* el);
        ~RtcStreamManager();
        //创建推流，返回offer
        int create_push_stream(uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id,  rtc::RTCCertificate* certificate, std::string& offer);
        PushStream* find_push_stream(const std::string& stream_name);
    private:
        EventLoop* el;
        std::unordered_map<std::string, PushStream*> _push_streams;
        
    };
} // namespace grtc

#endif //__RTC_STREAM_MANAGER_H_
