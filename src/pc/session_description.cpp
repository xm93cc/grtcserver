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
        //add feedback param
        audio_codec->feedback_param.push_back(FeedbackParam("transport-cc"));
        //add codec param
        audio_codec->codec_param["minptime"] = "10";
        audio_codec->codec_param["useinbandfec"] = "1";
        _codecs.push_back(audio_codec);
    }

    VideoContentDescription::VideoContentDescription(){
        auto codec = std::make_shared<VideoCodecInfo>();
        codec->clockrate = 90000; //视频采样率
        codec->id = 107;
        codec->name = "H264";
         //add feedback param
        codec->feedback_param.push_back(FeedbackParam("goog-remb"));
        codec->feedback_param.push_back(FeedbackParam("transport-cc"));
        codec->feedback_param.push_back(FeedbackParam("ccm", "fir"));
        codec->feedback_param.push_back(FeedbackParam("nack"));
        codec->feedback_param.push_back(FeedbackParam("nack", "pli"));
        // add codec param
        codec->codec_param["level-asymmetry-allowed"] = "1";
        codec->codec_param["packetization-mode"] = "1";
        codec->codec_param["profile-level-id"] = "42e01";
        auto rtx_codec = std::make_shared<VideoCodecInfo>();
        rtx_codec->id = 99;
        rtx_codec->clockrate = 90000;
        rtx_codec->name = "rtx";
        // add codec param
        rtx_codec->codec_param["apt"] = std::to_string(codec->id);
        _codecs.push_back(codec);
        _codecs.push_back(rtx_codec);
    }

    
    static void add_rtcp_fb_line(std::shared_ptr<CodecInfo> codec, std::stringstream& ss){
        for (auto param : codec->feedback_param)
        {
          ss << "a=rtcp-fb:" << codec->id << " " << param.id();
          if(!param.param().empty()){
            ss << " " << param.param();
          }
          ss << "\r\n";
        }
        
    }
    static void add_fmtp_line(std::shared_ptr<CodecInfo> codec, std::stringstream& ss){
        if(!codec->codec_param.empty()){
            ss << "a=fmtp:" << codec->id << " ";
            std::string data;
            for(auto param : codec->codec_param){
                data += (";" + param.first + "=" + param.second);
            }
            data = data.substr(1);
            ss << data << "\r\n";
        }
    }
    static void build_rtp_map(std::shared_ptr<MediaContentDescription> content, std::stringstream& ss){
        for (auto codec : content->get_codes())
        {
          ss << "a=rtpmap:" << codec->id << " " << codec->name << "/" << codec->clockrate;
          if(MediaType::MEDIA_TYPE_AUDIO == content->type()){
            auto audio_codec = codec->as_audio();
            ss << "/" << audio_codec->channels;
          }   
          ss << "\r\n";
          add_rtcp_fb_line(codec, ss);
          add_fmtp_line(codec, ss);
        }

        
    }

    static void build_rtp_direction(std::shared_ptr<MediaContentDescription> content, std::stringstream& ss){
            switch (content->direction())
            {
            case RtpDirection::k_send_recv:
                ss << "a=sendrecv\r\n";
                break;
             case RtpDirection::k_send_only:
                ss << "a=sendonly\r\n";
                break;
             case RtpDirection::k_recv_only:
                ss << "a=recvonly\r\n";
                break;
            
            default:
                ss << "a=inactive\r\n";
                break;
            }
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
            ss << "a=mid:" << content->mid() << "\r\n";
            build_rtp_direction(content, ss);
            if (content->rtcp_mux()){
                ss << "a=rtcp-mux\r\n";
            }
            build_rtp_map(content, ss);
        }
        return ss.str();
    }

} // namespace grtc
