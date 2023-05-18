#ifndef __RTC_WORKER_H_
#define __RTC_WORKER_H_
#include "base/event_loop.h"
#include "server/rtc_server.h"
#include "base/lock_free_queue.h"
#include "grtcserver_def.h"
#include"stream/rtc_stream_manager.h"
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
        LockFreeQueue<std::shared_ptr<RtcMsg>> _q_msg;
        std::unique_ptr<RtcStreamManager> _rtc_stream_mgr;
    private:
        //关闭描述符，事件循环，线程
        void _stop();
        //处理通知
        void _process_notify(int msg);
        //处理rtc消息
        void _process_rtc_msg();
        //处理push消息
        void _process_push(std::shared_ptr<RtcMsg> msg);
        //处理answer消息
        void _process_answer(std::shared_ptr<RtcMsg> msg);
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
        //发送Rtc msg
        int send_rtc_msg(std::shared_ptr<RtcMsg> msg);
        //推入RTC MSG
        void push_msg(std::shared_ptr<RtcMsg> msg);
        //弹出RTC MSG
        bool pop_msg(std::shared_ptr<RtcMsg>* msg);


        
        friend void rtc_worker_recv_notify(EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int /*events*/,void* data);
    };
    

} // namespace grtc

#endif //__RTC_WORKER_H_