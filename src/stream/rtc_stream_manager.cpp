//impl rtc_stream_manager.h

#include"stream/rtc_stream_manager.h"
#include"stream/push_stream.h"
namespace grtc
{
    RtcStreamManager::RtcStreamManager(EventLoop* el):el(el){}
    //free 
    RtcStreamManager::~RtcStreamManager(){}

     int RtcStreamManager::create_push_stream(uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id, std::string& offer)
    {
        PushStream* stream = new PushStream(el, uid, stream_name, audio, video, log_id, offer);


    }

    
} // namespace grtc
