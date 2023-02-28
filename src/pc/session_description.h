#ifndef __SESSION_DESCRIPTION_H_
#define __SESSION_DESCRIPTION_H_
#include <string>
#include <memory>
#include <vector>
#include "pc/codec_info.h"
namespace grtc
{
    enum class SdpType
    {
        k_offer = 0,
        k_answer = -1,
    };

    enum class MediaType
    {
        MEDIA_TYPE_AUDIO,
        MEDIA_TYPE_VIDEO
    };

    enum class RtpDirection{
        k_send_recv,
        k_send_only,
        k_recv_only,
        k_inactive
    };

    class MediaContentDescription
    {
    public:
        virtual ~MediaContentDescription() {}
        virtual MediaType type() = 0;
        virtual std::string mid() = 0;
        const std::vector<std::shared_ptr<CodecInfo>>& get_codes() const {
            return _codecs;
        }
        RtpDirection direction(){return _direction;}
        void set_direction(RtpDirection direction){_direction = direction;}
        bool rtcp_mux(){return _rtcp_mux;}
        void set_rtcp_mux(bool rtcp_mux){_rtcp_mux = rtcp_mux;}
    protected:
        std::vector<std::shared_ptr<CodecInfo>> _codecs;
        RtpDirection _direction;
        bool _rtcp_mux = true;
    };

    class AudioContentDescription : public MediaContentDescription
    {
    public:
        AudioContentDescription();
        MediaType type() override { return MediaType::MEDIA_TYPE_AUDIO; }
        std::string mid() override { return "audio"; }
    };

    class VideoContentDescription : public MediaContentDescription
    {
    public:
        VideoContentDescription();
        MediaType type() override { return MediaType::MEDIA_TYPE_VIDEO; }
        std::string mid() override { return "video"; }
    };

    // 内容组
    class ContentGroup
    {
    public:
        ContentGroup(const std::string &semantics) : _semantics(semantics) {}
        ~ContentGroup() {}
        std::string semantics() const { return _semantics; }
        inline const std::vector<std::string> &content_names() const { return _content_names; }
        void add_content_name(const std::string& content_name);
        bool has_content_name(const std::string& content_name);
    private:
        std::string _semantics;
        std::vector<std::string> _content_names;
    };


    class SessionDescription
    {
    private:
        SdpType _sdp_type;
        std::vector<std::shared_ptr<MediaContentDescription>> _contents;
        std::vector<ContentGroup> _content_groups;
    public:
        SessionDescription(SdpType type);
        ~SessionDescription();
        std::string to_string();
        // 添加m（媒体，分为音频，视频，数据）行
        void add_content(std::shared_ptr<MediaContentDescription> content);
        inline const std::vector<std::shared_ptr<MediaContentDescription>>& contents() const {return _contents;}
        void add_group(const ContentGroup& group);
        std::vector<const ContentGroup*>  get_group_by_name(const std::string& name) const;
    };

} // namespace grtc

#endif //__SESSION_DESCRIPTION_H_