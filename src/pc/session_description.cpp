// impl session_description.h
#include "pc/session_description.h"
#include <sstream>
namespace grtc
{

const char k_media_protocol_dtls_savpf[] = "UDP/TLS/RTP/SAVPF";
const char k_meida_protocol_savpf[] = "RTP/SAVPF";

    bool ContentGroup::has_content_name(const std::string& content_name){
        for(auto name : _content_names){
            if(name == content_name){
                return true;
            }
        }
        return false;
    }

    void ContentGroup::add_content_name(const std::string& content_name){
        if(!has_content_name(content_name)){
            _content_names.push_back(content_name);
        }
    }

    SessionDescription::SessionDescription(SdpType type) : _sdp_type(type)
    {
    }

    SessionDescription::~SessionDescription()
    {
    }

    void SessionDescription::add_group(const ContentGroup& group){
        _content_groups.push_back(group);
    }
    void SessionDescription::add_content(std::shared_ptr<MediaContentDescription> content)
    {
        _contents.push_back(content);
    }

    std::vector<const ContentGroup*>  SessionDescription::get_group_by_name(const std::string& name) const{
        std::vector<const ContentGroup*> content_group;
        for(const ContentGroup& group : _content_groups){
            if(group.semantics() == name){
                content_group.push_back(&group);
            }
        }
        return content_group;
    }

    AudioContentDescription::AudioContentDescription(){
        auto audio_codec = std::make_shared<AudioCodecInfo>();
        audio_codec->id = 111;
        audio_codec->channels = 2;//双声道
        audio_codec->clockrate = 48000; //音频采样
        audio_codec->name = "opus";
        _codecs.push_back(audio_codec);
    }

    VideoContentDescription::VideoContentDescription(){
        auto codec = std::make_shared<VideoCodecInfo>();
        codec->clockrate = 90000; //视频采样率
        codec->id = 107;
        codec->name = "H264";

        auto rtx_codec = std::make_shared<VideoCodecInfo>();
        rtx_codec->id = 99;
        rtx_codec->clockrate = 90000;
        rtx_codec->name = "rtx";

        _codecs.push_back(codec);
        _codecs.push_back(rtx_codec);
    }
    std::string SessionDescription::to_string()
    {
        std::stringstream ss;
        ss << "v=0\r\n";
        // session origin
        // RFC 4566
        // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
        ss << "o=- 0 2 IN IP4 127.0.0.1\r\n";
        // session name
        ss << "s=-\r\n";
        // time description
        ss << "t=0 0\r\n";

        //BUNDLE 信息
        std::vector<const ContentGroup*> content_group = get_group_by_name("BUNDLE");
        if(!content_group.empty()){
            ss << "a=group:BUNDLE";
            for(auto group : content_group){
                for(auto content_name : group->content_names()){
                    ss << " " << content_name;
                }
            }
            ss << "\r\n";
        }

        //RFC 4566
        //m=<media> <port> <proto> <fmt>
         ss << "a=msid-semantic: WMS\r\n";
        for(auto content : _contents){
            std::string fmt;
            for(auto codec : content->get_codes()){
                fmt.append(" ");
                fmt.append(std::to_string(codec->id));
            }
            
            ss << "m=" << content->mid() << " 9 " << k_media_protocol_dtls_savpf << fmt << "\r\n";
            ss << "c=IN IP4 0.0.0.0\r\n";
            ss << "a=rtcp:9 IN IP4 0.0.0.0\r\n";
        }
        return ss.str();
    }

} // namespace grtc
