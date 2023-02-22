#ifndef __RTC_STREAM_MANAGER_H_
#define __RTC_STREAM_MANAGER_H_
#include<string>
#include"base/event_loop.h"
namespace grtc
{
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
                                , bool audio, bool video, uint32_t log_id, std::string& offer);
    
    private:
        EventLoop* el;
        
    };
} // namespace grtc

#endif //__RTC_STREAM_MANAGER_H_
