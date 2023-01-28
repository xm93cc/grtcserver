// impl SignalingWorker.h
#include"server/signaling_worker.h"
#include<rtc_base/logging.h>
#include<unistd.h>

namespace grtc
{

    void signaling_worker_recv_notify (EventLoop* el,IOWatcher* w,int fd, int events,void* data){
        int msg;
        if(read(fd,&msg,sizeof(msg)) == sizeof(int)){
           
            return;
        }
        SignalingWorker* worker = (SignalingWorker*)data;
        worker->_process_notify(msg);
        
    }
    SignalingWorker::SignalingWorker(int worker_id):_worker_id(worker_id),_el(new EventLoop(this)){}
    SignalingWorker::~SignalingWorker(){}

    int SignalingWorker::init(){
        int fds[2];
        if(pipe(fds)){
            RTC_LOG(LS_WARNING)<<"create pipe error. error: "<< strerror(errno) << " errno: "<<errno;
            return -1;
        }
        _notify_recv_fd = fds[0];
        _notify_send_fd = fds[1];
        _pipe_watcher = _el->create_io_event(signaling_worker_recv_notify, this);
        //开始事件
        _el->strart_io_event(_pipe_watcher,_notify_recv_fd,EventLoop::READ);
        return 0;
    }


    bool SignalingWorker::start(){
        if(_thread){
           RTC_LOG(LS_WARNING) << "signaling worker already start." ;
           return false;
        }
        _thread = new std::thread([=](){
            RTC_LOG(LS_INFO) << "signaling worker thread start up. ";
            _el->start();
            RTC_LOG(LS_INFO) << "signaling worker thread stop. ";
        });

        return true;
    }

    void SignalingWorker::_process_notify(int msg){
        switch (msg)
        {
        case QUIT:
            _stop();
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: "<<msg;
            break;
        }
    }

    int SignalingWorker::notify(int msg){
        int ret = write(_notify_send_fd,&msg,sizeof(int));
        return ret == sizeof(msg) ? 0 : -1;
    }

    //内部stop方法释放资源 关闭文件描述符
    void SignalingWorker::_stop(){
        if(!_thread){
            RTC_LOG(LS_WARNING) << "signaling worker not running.";
            return;
        }
        _el->delete_io_event(_pipe_watcher);
        _el->stop();
        close(_notify_recv_fd);
        close(_notify_send_fd);  
    }

    void SignalingWorker::stop(){
        notify(QUIT);
    }

    void SignalingWorker::join(){
        if(_thread && _thread->joinable()){
            _thread->join();
        }
    }

   int SignalingWorker::notify_new_conn(int fd){

   } 

} // namespace grtc
