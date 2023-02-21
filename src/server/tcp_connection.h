#ifndef __TCP_CONNECTION_H_
#define __TCP_CONNECTION_H_
#include"base/event_loop.h"
#include"base/ghead.h"
#include<list>
#include<rtc_base/slice.h>
#include<rtc_base/sds.h>
namespace grtc
{
    class TcpConnection
    {
    public:
        /* data */
        int fd;
        char ip[64];
        int port;
        IOWatcher* io_watcher = nullptr;
        TimerWatcher* timer_watcher  = nullptr;
        EventLoop* _el;
        //redis中的buffer结构，sds 有较为全面操作的方法
        sds querybuf;
        //预期读取字节数
        size_t bytes_expected = GHEAD_SIZE;
        //已处理字节数
        size_t bytes_processed = 0;
        int current_state = STATE_HEAD;
        unsigned long last_interaction = 0;
        std::list<rtc::Slice> reply_list;
        size_t cur_resp_pos = 0;
        


    public:
        enum{
            STATE_HEAD = 0,
            STATE_BODY = 1
        };
        TcpConnection(int fd);
        ~TcpConnection();
    };

} // namespace grtc


#endif //__TCP_CONNECTION_H_