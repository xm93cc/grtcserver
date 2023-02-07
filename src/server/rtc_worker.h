#ifndef __RTC_WORKER_H_
#define __RTC_WORKER_H_
#include "base/event_loop.h"
#include "server/rtc_server.h"
namespace grtc
{
    class RtcWorker
    {
    private:
        int _worker_id;
        EventLoop* _el;
        std::thread* _thread = nullptr;
        IOWatcher* _pipe_watcher = nullptr;
        int _notify_recv_fd = -1;
        int _notify_send_fd = -1;
        RtcServerOptions _options;
    private:
        //关闭描述符，事件循环，线程
        void _stop();
        //处理通知
        void _process_notify(int msg);
    public:
        enum{
            QUIT = 0,
            RTC_MSG = 1
        }; 
    public:
        RtcWorker(int worker_id,const RtcServerOptions& options);
        ~RtcWorker();
        void stop();
        void join();
        int init();
        int notify(int msg);
        bool start();
        friend void rtc_worker_recv_notify(EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int /*events*/,void* data);
    };
    

} // namespace grtc

#endif //__RTC_WORKER_H_