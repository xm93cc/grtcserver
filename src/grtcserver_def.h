#ifndef __GRTCSERVER_DEF_H_
#define __GRTCSERVER_DEF_H_

#define CMDNO_PUSH     1
#define CMDNO_PULL     2
#define CMDNO_ANSWER   3
#define CMDNO_STOPPUSH 4
#define CMDNO_STOPPULL 5
#define MAX_RES_BUF 4096

namespace grtc
{
    struct RtcMsg{
        int cmdno = -1;
        uint64_t uid = 0;
        std::string stream_name;
        int audio = 0;
        int video = 0;
        uint32_t log_id = 0;
        void* worker = nullptr;
        void* conn = nullptr;
        std::string sdp;
        int err_no = 0;
        int fd = 0;
    };
} // namespace grtc


#endif //__GRTCSERVER_DEF_H_