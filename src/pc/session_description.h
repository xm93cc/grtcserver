#ifndef __SESSION_DESCRIPTION_H_
#define __SESSION_DESCRIPTION_H_
#include<string>

namespace grtc
{
    enum class SdpType{
        k_offer = 0,
        k_answer = -1,
    };
    class SessionDescription
    {
    private:
        SdpType _sdp_type;
    public:
        SessionDescription(SdpType type);
        ~SessionDescription();
        std::string to_string();
    };
   
} // namespace grtc

#endif //__SESSION_DESCRIPTION_H_