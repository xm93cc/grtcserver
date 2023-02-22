// impl rtc_stream.h
#include "stream/rtc_stream.h"
namespace grtc
{
    RtcStream::RtcStream(EventLoop *el, uint64_t uid, const std::string &stream_name
                        , bool audio, bool video, uint32_t log_id, std::string &offer):
                        el(el), uid(uid), stream_name(stream_name), audio(audio), video(video), log_id(log_id), offer(offer)
    {
    }
    RtcStream::~RtcStream()
    {
    }
} // namespace grtc
