// impl push_stream.h
#include "stream/push_stream.h"
namespace grtc
{
    PushStream::PushStream(EventLoop* el,uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id):
                          RtcStream(el, uid, stream_name, audio, video, log_id)
    {
    }
    //free
    PushStream::~PushStream(){}

    std::string PushStream::create_offer(){
        return pc->create_offer();
    }
} // namespace grtc