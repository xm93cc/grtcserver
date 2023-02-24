// impl session_description.h
#include "pc/session_description.h"
#include <sstream>
namespace grtc
{

    SessionDescription::SessionDescription(SdpType type) : _sdp_type(type)
    {
    }

    SessionDescription::~SessionDescription()
    {
    }

    void SessionDescription::add_content(std::shared_ptr<MediaContentDescription> content)
    {
        _contents.push_back(content);
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
        return ss.str();
    }

} // namespace grtc
