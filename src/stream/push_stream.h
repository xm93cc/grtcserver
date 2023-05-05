#ifndef __PUSH_STREAM_H_
#define __PUSH_STREAM_H_
#include"stream/rtc_stream.h"
namespace grtc
{
    class PushStream : public RtcStream{
    public:
         PushStream(EventLoop* el,PortAllocator* allocator ,uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id);
         ~PushStream() override;
         //创建offer
         std::string create_offer() override;
    };
} // namespace grtc

#endif //__PUSH_STREAM_H_