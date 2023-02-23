//impl session_description.h
#include "pc/session_description.h"
#include<sstream>
namespace grtc
{
     
    SessionDescription::SessionDescription(SdpType type):_sdp_type(type)
    {
    }
    
    SessionDescription::~SessionDescription()
    {
    }

    std::string SessionDescription::to_string(){
        std::stringstream ss;
        ss << "v=0\r\n";
        return ss.str();
    }
    
} // namespace grtc
