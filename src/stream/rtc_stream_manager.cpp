//impl rtc_stream_manager.h
#include <rtc_base/rtc_certificate.h>
#include"stream/rtc_stream_manager.h"
#include"stream/push_stream.h"
#include "base/conf.h"
extern grtc::GeneralConf* g_conf;
namespace grtc
{
    RtcStreamManager::RtcStreamManager(EventLoop* el)
    :el(el),_allocator(new PortAllocator())
    {
        _allocator->set_port_range(g_conf->ice_min_port, g_conf->ice_max_port);
    }
    //free 
    RtcStreamManager::~RtcStreamManager(){}

     int RtcStreamManager::create_push_stream(uint64_t uid, const std::string& stream_name
                                , bool audio, bool video, uint32_t log_id, rtc::RTCCertificate* certificate , std::string& offer)
    {
        PushStream* stream = find_push_stream(stream_name);
        if(stream){
            _push_streams.erase(stream_name);
            delete stream;
        }
        stream = new PushStream(el,_allocator.get(), uid, stream_name, audio, video, log_id);
        stream->start(certificate);
        offer = stream->create_offer();
        return 0;

    }


     PushStream* RtcStreamManager::find_push_stream(const std::string& stream_name){
            auto iter = _push_streams.find(stream_name);
            if(iter != _push_streams.end()){
                return iter->second;
            }
            return nullptr;
     }

    
} // namespace grtc
