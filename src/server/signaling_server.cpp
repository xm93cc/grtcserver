#include "server/signaling_server.h"
#include <yaml-cpp/yaml.h>
#include <rtc_base/logging.h>
#include "base/socket.h"
#include <unistd.h>
#include "server/signaling_worker.h"
namespace grtc
{

    void signaling_server_recv_notify(EventLoop */*el*/, IOWatcher */*w*/, int fd, int /*events*/, void *data)
    {
        int msg;
        if(read(fd,&msg,sizeof(int)) != sizeof(int)){
            RTC_LOG(LS_WARNING)<<"read error errno: "<< errno << " error: "<<strerror(errno);
        }
        SignalingServer* server = (SignalingServer*)data;
        server->_process_notify(msg); 
    }

    void accept_new_conn(EventLoop */*el*/, IOWatcher */*w*/, int fd, int /*events*/, void *data)
    {
        int cfd;
        char cip[128];//or IPV6
        int cport;

        cfd = tcp_accept(fd,cip,&cport);
        if(-1 == cfd){
            return;
        }
        RTC_LOG(LS_INFO) << "accept new connection. fd: "<< cfd << " host: "<< cip << " port: " << cport;
        SignalingServer* server = (SignalingServer*)data;
        server->_dispacth_new_conn(cfd);
    }

    // init EventLoop
    SignalingServer::SignalingServer() : _el(new EventLoop(this))
    {
    }
    // free resoure
    SignalingServer::~SignalingServer()
    {
        if(_el){
            delete _el;
            _el = nullptr;
        }

        if(_thread){
            delete _thread;
            _thread = nullptr;
        }
        for(auto worker: _workers){
            if(worker)
                delete worker;
        }
        _workers.clear();

    }

    int SignalingServer::init(const char *conf_path)
    {
        if (!conf_path)
        {
            RTC_LOG(LS_WARNING) << "signaling server conf path is null";
            return -1;
        }
        try
        {
            YAML::Node config = YAML::LoadFile(conf_path);
            _options.host = config["host"].as<std::string>();
            _options.port = config["port"].as<int>();
            _options.connection_timeout = config["connection_timeout"].as<int>();
            _options.worker_num = config["worker_num"].as<int>();
        }
        catch (YAML::Exception *e)
        {
            RTC_LOG(LS_WARNING) << "catch exception:  " << e->msg << " ,line: " << e->mark.line << " ,column: " << e->mark.column;
            return -1;
        }

        int fd[2];
        if (pipe(fd))
        {
            RTC_LOG(LS_WARNING) << "create pipe error . errno: " << errno << " error: " << strerror(errno);
            return -1;
        }
        //注冊管道读取事件
        _notify_recv_fd = fd[0];
        _notify_send_fd = fd[1];
        _pipe_watcher = _el->create_io_event(signaling_server_recv_notify, this);
        _el->strart_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
        //注册tcp连接事件
        _listen_fd = create_tcp_server(_options.host.c_str(), _options.port);
        if(-1 == _listen_fd){
            RTC_LOG(LS_WARNING) << "listen fd error ,errno" << errno << strerror(errno);
            return -1;
        }
        _io_watcher = _el->create_io_event(accept_new_conn,this);
        _el->strart_io_event(_io_watcher,_listen_fd,EventLoop::READ);

        // create worker 
        for (int i = 0; i < _options.worker_num; i++)
        {
               if(_create_worker(i) != 0){
                return -1;
               } 
        }
        return 0;
    }

    void SignalingServer::_process_notify(int msg){
        switch (msg)
        {
        case QUIT:
            _stop();
            break;
        
        default:
            RTC_LOG(LS_WARNING)<< "unknown msg: "<< msg;
            break;
        }
    }

    void SignalingServer::_stop(){
        if(!_thread){
            RTC_LOG(LS_WARNING)<<"signaling server not running.";
            return ;
        }
        _el->delete_io_event(_io_watcher);
        _el->delete_io_event(_pipe_watcher);
        _el->stop();

        close(_notify_recv_fd);
        close(_notify_send_fd);
        close(_listen_fd);
        RTC_LOG(LS_INFO)<<"signaling server stop.";    
        for(auto worker : _workers){
            if(worker){
                worker->stop();
                worker->join();
            }
        }
        RTC_LOG(LS_INFO)<<"signaling worker stop deno. "; 
        
    }

    void SignalingServer::stop(){
        notify(SignalingServer::QUIT);
    }

    int SignalingServer::notify(int msg){
        int ret = write(_notify_send_fd,&msg,sizeof(msg));
        return ret == sizeof(int) ? 0 : -1;
    }
    void SignalingServer::join(){
        if(_thread && _thread->joinable()){
            _thread->join();
        }
    }

    bool SignalingServer::start(){
        if(_thread){
            RTC_LOG(LS_WARNING)<<"signaling server already start.";
            return false;
        }
        _thread = new std::thread([=](){
            RTC_LOG(LS_INFO) << "signaling server event loop run.";
            _el->start();
            RTC_LOG(LS_INFO) << "signaling server event loop stop.";
        });
        return true;
    }

    int SignalingServer::_create_worker(int worker_id){
        RTC_LOG(LS_INFO) << "signaling server create worker . worker id :  "<< worker_id ;
        SignalingWorker* worker = new SignalingWorker(worker_id,_options);
        if(worker->init() != 0 ){
            return -1;
        }

        if(!worker->start()){
            return -1;
        }
        _workers.push_back(worker);
        return 0;
    }

    void SignalingServer::_dispacth_new_conn(int fd){
        int index = _next_worker_index;
        _next_worker_index++;
        if((size_t)_next_worker_index >= _workers.size()){
            _next_worker_index = 0;
        }
        SignalingWorker* worker = _workers[index];
        worker->notify_new_conn(fd);
    }
 
} // namespace grtc