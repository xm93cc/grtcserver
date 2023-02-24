#ifndef __SESSION_DESCRIPTION_H_
#define __SESSION_DESCRIPTION_H_
#include <string>
#include <memory>
#include <vector>
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

    class MediaContentDescription
    {
    public:
        virtual ~MediaContentDescription() {}
        virtual MediaType type() = 0;
        virtual std::string mid() = 0;
    };

    class AudioContentDescription : public MediaContentDescription
    {
    public:
        MediaType type() override { return MediaType::MEDIA_TYPE_AUDIO; }
        std::string mid() override { return "audio"; }
    };

    class VideoContentDescription : public MediaContentDescription
    {
    public:
        MediaType type() override { return MediaType::MEDIA_TYPE_VIDEO; }
        std::string mid() override { return "video"; }
    };
    class SessionDescription
    {
    private:
        SdpType _sdp_type;
        std::vector<std::shared_ptr<MediaContentDescription>> _contents;
        

    public:
        SessionDescription(SdpType type);
        ~SessionDescription();
        std::string to_string();
        //添加m（媒体，分为音频，视频，数据）行
        void add_content(std::shared_ptr<MediaContentDescription> content);
    };

} // namespace grtc

#endif //__SESSION_DESCRIPTION_H_