#ifndef __SIGNALING_WORKER_H_
#define __SIGNALING_WORKER_H_

#include "base/event_loop.h"
#include "base/lock_free_queue.h"
#include<rtc_base/slice.h>
#include<vector>
#include<thread>
#include"server/signaling_server.h"
#include<json/json.h>
#include<queue>
#include<mutex>
#include"grtcserver_def.h"
namespace grtc{
class TcpConnection;
class SignalingWorker{
    private:
        void _stop();
        void _process_notify(int msg);
        void _new_conn(int fd);
        void _read_query(int fd);
        //处理buffer
        int _process_query_buffer(TcpConnection* c);
        //处理请求过来的数据报
        int _process_request(TcpConnection* c, const rtc::Slice& header,const rtc::Slice& body);
        //关闭连接
        void  _close_conn(TcpConnection* c);
        //从连接数组中移除连接
        void _remove_conn(TcpConnection* c);
        //连接建立超过配置文件配置的时长后关闭连接
        void _process_timeout(TcpConnection* c);
        //处理推流
        int _process_push(int cmdno,TcpConnection* c,const Json::Value& root,uint32_t log_id);
        //响应服务端offer
        void  _response_server_offer(std::shared_ptr<RtcMsg> msg);
        //在连接中添加报文
        void _add_reply(TcpConnection* c, const rtc::Slice& reply);
        //写回报文
        void _write_reply(int fd);
        //处理answer
        int _process_answer(int cmdno, TcpConnection *c, const Json::Value &root, uint32_t log_id);
    
    public:
        enum{
            QUIT = 0,
            NEW_CONN = 1,
            RTC_MSG = 2
        };
        SignalingWorker(int worker_id,SignalingServerOptions& options);
        ~SignalingWorker();
        void stop();
        void join();
        int init();
        int notify(int msg);
        bool start();
        int notify_new_conn(int fd);
        //将rtc msg 从其他历程推送到signaling worker
        int send_rtc_msg(std::shared_ptr<RtcMsg> msg);
        //向队列push msg
        void push_msg(std::shared_ptr<RtcMsg> msg);
        //从队列弹出 msg
        std::shared_ptr<RtcMsg> pop_msg();
        //处理队列中的RtcMsg
        void _process_rtc_msg();
        friend void signaling_worker_recv_notify(EventLoop* el,IOWatcher* w,int fd, int events,void* data);
        friend void conn_io_cb (EventLoop* el,IOWatcher* w,int fd, int events,void* data);
        friend void conn_timeout_cb(EventLoop* el,TimerWatcher* w,void* data);
      private:
        int _worker_id;
        EventLoop* _el;
        std::thread* _thread = nullptr;
        IOWatcher* _pipe_watcher = nullptr;
        int _notify_recv_fd = -1;
        int _notify_send_fd = -1;
        LockFreeQueue<int> _q_conn;
        std::vector<TcpConnection*> _conns;
        SignalingServerOptions _options;
        std::queue<std::shared_ptr<RtcMsg>> _q_msg;
        std::mutex _q_msg_mtx;
};


}// namespace grtc

#endif // __SIGNALING_WORKER_H_