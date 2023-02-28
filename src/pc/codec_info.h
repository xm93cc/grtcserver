#ifndef __CODEC_INFO_H_
#define __CODEC_INFO_H_
#include <string>
#include <map>
#include <vector>
namespace grtc
{
    class AudioCodecInfo;
    class VideoCodecInfo;
    class FeedbackParam{
        private:
            std::string _id;
            std::string _param;
        public:
            FeedbackParam(const std::string& id, const std::string& param): _id(id), _param(param){}
            FeedbackParam(const std::string& id): _id(id), _param(""){}
            std::string id(){return _id;}
            std::string param(){return _param;}
    };

    typedef std::map<std::string, std::string> CodecParam;
    // def codec base class
    class CodecInfo
    {
    public:
        int id;
        std::string name;
        int clockrate;
        std::vector<FeedbackParam> feedback_param;
        CodecParam codec_param;
    public:
        virtual AudioCodecInfo *as_audio() { return nullptr; }
        virtual VideoCodecInfo *as_video() { return nullptr; }
    };
    class AudioCodecInfo : public CodecInfo
    {
    public:
        int channels;

    public:
        AudioCodecInfo *as_audio() override { return this; }
    };

    class VideoCodecInfo : public CodecInfo
    {
    public:
        VideoCodecInfo *as_video() override { return this; }
    };
} // namespace grtc

#endif // __CODEC_INFO_H_
