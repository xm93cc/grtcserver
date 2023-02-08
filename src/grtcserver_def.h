#ifndef __GRTCSERVER_DEF_H_
#define __GRTCSERVER_DEF_H_

#define CMDNO_PUSH     1
#define CMDNO_PULL     2
#define CMDNO_ANSWER   3
#define CMDNO_STOPPUSH 4
#define CMDNO_STOPPULL 5

namespace grtc
{
    struct RtcMsg{
        int cmdno = -1;
        uint64_t uid = 0;
        std::string stream_name;
        int audio = 0;
        int video = 0;
        uint32_t log_id = 0;
    };
} // namespace grtc


#endif //__GRTCSERVER_DEF_H_