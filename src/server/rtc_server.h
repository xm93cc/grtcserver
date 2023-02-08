#ifndef __RTC_SERVER_H_
#define __RTC_SERVER_H_
#include "base/event_loop.h"
#include "server/signaling_worker.h"
#include "grtcserver_def.h"
#include <queue>
#include <mutex>
namespace grtc
{
    struct RtcServerOptions
    {
        int worker_num;
    };

    class RtcWorker;
    
    class RtcServer
    {
    public:
        enum{
            QUIT = 0,
            RTC_MSG = 1
        };
        RtcServer(/* args */);
        ~RtcServer();
        //读取配置文件，初始化
        int init(const char* conf_file);
        //结束事件循环，停止线程
        void stop();
        //join _thread
        void join();
        //开始事件循环，开始线程
        bool start();
        //发送通知
        int notify(int msg);
        //管道发送rtc msg
        int send_rtc_msg(std::shared_ptr<RtcMsg> msg);
        //向队列推入消息
        void push_msg(std::shared_ptr<RtcMsg> msg); 
        //弹出消息
        std::shared_ptr<RtcMsg> pop_msg();
        friend void rtc_server_recv_notify(EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int /*events*/,void* data);
    private:
        //处理通知(管道)
        void _process_notify(int msg);
        //停止事件循环，关闭管道描述符
        void _stop();
        //处理rtc msg
        void _process_rtc_msg();
        //创建工作worker
        int _create_worker(int worker_id);
        //通过stream_name hash的方式获取worker
        RtcWorker* _get_worker(const std::string& stream_name);
        
    private:
        EventLoop *_el;
        RtcServerOptions _options;
        IOWatcher* _pipe_watcher = nullptr;
        IOWatcher* _io_watcher = nullptr;
        int _notify_recv_fd = -1;
        int _notify_send_fd = -1;
      //  std::vector<SignalingWorker*> _workers;
        std::thread* _thread = nullptr; 
        std::queue<std::shared_ptr<RtcMsg>> _q_msg;
        std::mutex _q_msg_mtx;
        std::vector<RtcWorker*> _workers;

    };
} // namespace grtc

#endif //__RTC_SERVER_H_