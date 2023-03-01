// impl ice_credentials.h
#include<rtc_base/helpers.h>
#include "ice/ice_credentials.h"
#include "ice/ice_def.h"

namespace grtc
{
    IceParameters IceCredentials::create_random_ice_credentials(){
        return IceParameters(rtc::CreateRandomString(ICE_UFRAG_LENGTH), rtc::CreateRandomString(ICE_PWD_LENGTH));
    }
} // namespace grtc
