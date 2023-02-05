#ifndef __BASE_XHEAD_H_
#define __BASE_XHEAD_H_
#include <stdint.h>
namespace grtc
{
    const int GHEAD_SIZE = 36;
    const uint32_t GHEAD_MAGIC_NUM = 0xfb202202;
    struct ghead_t
    {
        uint16_t id;
        uint16_t version;
        uint32_t log_id;
        char provider[16];
        uint32_t magic_num;
        uint32_t reserved;
        uint32_t body_len;
    };

} // namespace grtc

#endif //__BASE_XHEAD_H_