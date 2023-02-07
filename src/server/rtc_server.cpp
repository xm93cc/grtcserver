#include "server/rtc_server.h"
#include <yaml-cpp/yaml.h>
#include <rtc_base/logging.h>
#include <unistd.h>
#include "server/rtc_worker.h"
namespace grtc
{
    RtcServer::RtcServer():_el(new EventLoop(this)){}
    RtcServer::~RtcServer(){
      if(_el){
         delete _el;
         _el = nullptr;
      }
      if(_thread){
         delete _thread;
         _thread = nullptr;
      }
      for(auto worker : _workers){
        if(worker){
             delete worker;
        }
      }
      _workers.clear();
    }
    
    void rtc_server_recv_notify(EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int /*events*/,void* data){
        int msg;
        if(read(fd,&msg,sizeof(int)) != sizeof(int)){
            RTC_LOG(LS_WARNING)<<"read error errno: "<< errno << " error: "<<strerror(errno);
        }
        RtcServer* server = (RtcServer*)data;
        server->_process_notify(msg); 
    }

    int RtcServer::init(const char* conf_path){
        if(!conf_path){
            RTC_LOG(LS_WARNING) << "conf_file is null";
            return -1;
        }
        try{
            YAML::Node config = YAML::LoadFile(conf_path);
            _options.worker_num = config["worker_num"].as<int>();
        }catch(YAML::Exception& e){
            RTC_LOG(LS_WARNING) << "rtc server load conf file error: " << e.msg;
            return -1;
        }
        //创建管道
        int fds[2];
        if(pipe(fds)){
            RTC_LOG(LS_WARNING)<<"create pipe error. error: "<< strerror(errno) << " errno: "<<errno;
            return -1;
        }
        _notify_recv_fd = fds[0];
        _notify_send_fd = fds[1];
        _pipe_watcher = _el->create_io_event(rtc_server_recv_notify, this);
        _el->strart_io_event(_pipe_watcher,_notify_recv_fd,EventLoop::READ);
        for(int i = 0; i < _options.worker_num; i++){
            if(_create_worker(i) != 0){
                return -1;
            }
        }
        return 0;
    }

    int RtcServer::_create_worker(int worker_id){
        RTC_LOG(LS_INFO) << "rtc server create worker, worker_id: " << worker_id;
        RtcWorker* worker = new RtcWorker(worker_id,_options);
        if(worker->init() != 0){
            return -1;
        }
        if(!worker->start()){
            return -1;
        }
        _workers.push_back(worker);
        return 0;
    }

    void RtcServer::_process_notify(int msg){
        switch (msg)
        {
        case RtcServer::QUIT:
             _stop();
            break;
        case RtcServer::RTC_MSG:
             _process_rtc_msg();
            break;
        
        default:
            RTC_LOG(LS_WARNING)<< "unknown msg: "<< msg;
            break;
        }
    }

    void RtcServer::_stop(){
         if(!_thread){
            RTC_LOG(LS_WARNING)<<"signaling server not running.";
            return ;
        }
        //_el->delete_io_event(_io_watcher);
        _el->delete_io_event(_pipe_watcher);
        _el->stop();

        close(_notify_recv_fd);
        close(_notify_send_fd);
        RTC_LOG(LS_INFO)<<"rtc server stop.";    
        for(auto worker : _workers){
            if(worker){
                worker->stop();
                worker->join();
            }
        }
        RTC_LOG(LS_INFO)<<"rtc worker stop deno. "; 
    }

    void RtcServer::join(){
        if(_thread && _thread->joinable()){
            _thread->join();
        }
    }

    bool RtcServer::start(){
        if (_thread)
        {
            RTC_LOG(LS_WARNING) << "rtc server already start.";
            return false;
        }
        _thread = new std::thread([=](){
            RTC_LOG(LS_INFO) << "rtc server event loop run.";
            _el->start();
            RTC_LOG(LS_INFO) << "rtc server event loop stop.";
        });
        return true;
        
    }
    int RtcServer::notify(int msg){
        int ret = write(_notify_send_fd,&msg,sizeof(int));
        return ret == sizeof(int) ? 0 : -1;
    }

    void RtcServer::stop(){
       notify(RtcServer::QUIT); 
    }

    void RtcServer::push_msg(std::shared_ptr<RtcMsg> msg){
        std::unique_lock<std::mutex> lock(_q_msg_mtx);
        _q_msg.push(msg);
    }

    std::shared_ptr<RtcMsg> RtcServer::pop_msg(){
        std::unique_lock<std::mutex> lock(_q_msg_mtx);
        if(_q_msg.empty()){
            return nullptr;
        }
        std::shared_ptr<RtcMsg> msg = _q_msg.front();
        _q_msg.pop();
        return msg;  
    }

    int RtcServer::send_rtc_msg(std::shared_ptr<RtcMsg> msg){
        push_msg(msg);
        return notify(RtcServer::RTC_MSG);
    }

    void RtcServer::_process_rtc_msg(){
        std::shared_ptr<RtcMsg> msg = pop_msg();
        if(!msg){
            return;
        }
        RTC_LOG(LS_WARNING) << "============cmdno: " << msg->cmdno << " , uid: " << msg->uid;
    }

    
} // namespace grtc
