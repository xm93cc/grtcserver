#ifndef __SIGNALING_WORKER_H_
#define __SIGNALING_WORKER_H_

#include "base/event_loop.h"
#include<thread>
namespace grtc{

class SignalingWorker{
    private:
        int _worker_id;
        EventLoop* _el;
        std::thread* _thread;
        IOWatcher* _pipe_watcher = nullptr;
        int _notify_recv_fd = -1;
        int _notify_send_fd = -1;

    private:
        void _stop();
        void _process_notify(int msg);
    
    public:
        enum{
            QUIT = 0,
            NEW_CONN = 1
        };
        SignalingWorker(int worker_id);
        ~SignalingWorker();
        void stop();
        void join();
        int init();
        int notify(int msg);
        bool start();
        int notify_new_conn(int fd);
        friend void signaling_worker_recv_notify(EventLoop* el,IOWatcher* w,int fd, int events,void* data);
};


}// namespace grtc

#endif // __SIGNALING_WORKER_H_