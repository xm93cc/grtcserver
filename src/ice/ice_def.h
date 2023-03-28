#ifndef __ICE_DEF_H_
#define __ICE_DEF_H_
namespace grtc
{
    extern const int ICE_UFRAG_LENGTH;
    extern const int ICE_PWD_LENGTH;

    enum IceCandidateComponent{
        RTP = 1,
        RTCP = 2
    };
} // namespace grtc

#endif //__ICE_DEF_H_