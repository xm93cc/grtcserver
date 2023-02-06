// impl SignalingWorker.h
#include"server/signaling_worker.h"
#include<rtc_base/logging.h>
#include<unistd.h>
#include"base/socket.h"
#include"server/tcp_connection.h"
namespace grtc
{

    void signaling_worker_recv_notify (EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int /*events*/,void* data){
        int msg;
        if(read(fd,&msg,sizeof(msg)) != sizeof(int)){
            return;
        }
        SignalingWorker* worker = (SignalingWorker*)data;
        worker->_process_notify(msg);
        
    }
    SignalingWorker::SignalingWorker(int worker_id,SignalingServerOptions& options):_worker_id(worker_id),_el(new EventLoop(this)),_options(options){}
    SignalingWorker::~SignalingWorker(){
        for(auto conn : _conns){
            if(conn){
                _close_conn(conn);
            }
        }
        _conns.clear();
        if(_el){
            delete _el;
            _el = nullptr;
        }

        if(_thread){
            delete _thread;
            _thread = nullptr;
        }



    }

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
            RTC_LOG(LS_INFO) << "signaling worker thread event loop start up. ";
            _el->start();
            RTC_LOG(LS_INFO) << "signaling worker thread event loop stop. ";
        });

        return true;
    }

    void SignalingWorker::_process_notify(int msg){
        RTC_LOG(LS_INFO)<<"process notify : "<<msg; 
        switch (msg)
        {
        case SignalingWorker::QUIT:
            _stop();
            break;
        case SignalingWorker::NEW_CONN:
            int fd;
            if(_q_conn.consume(&fd)){
                _new_conn(fd);
            }
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown msg: "<<msg;
            break;
        }
    }

    void SignalingWorker::_read_query(int fd){
        RTC_LOG(LS_INFO)<< "signaling worker "<< _worker_id << " receive read event, fd: " << fd;
        if(fd < 0 || (size_t)fd >= _conns.size()){
            RTC_LOG(LS_WARNING) << "invalid fd: "<< fd ;
            return;
        }

        TcpConnection* c = _conns[fd];
        int nread = 0;
        //第一次读取头部大小的数据报
        int read_len = c->bytes_expected;
        //buffer 中已读取多少字节
        int qb_len = sdslen(c->querybuf);
        //根据本次需要读取的长度 创建or扩充buffer
        c->querybuf = sdsMakeRoomFor(c->querybuf,read_len);
        nread = sock_read_data(fd,c->querybuf + qb_len,read_len);
        c->last_interaction = _el->now();
        RTC_LOG(LS_INFO) << "sock read data , len: " << nread;
        if(-1 == nread){
            //case -1 connection close 
            _close_conn(c);
            return;
        }else if(nread > 0){
            //更改buffer已读取的长度，方便sdslen读取有效的长度
            sdsIncrLen(c->querybuf,nread);
        }
        int ret = _process_query_buffer(c);
        if(ret != 0){
            _close_conn(c);
            return;
        }

    }

    int SignalingWorker::_process_query_buffer(TcpConnection *c){
        while (sdslen(c->querybuf) >= c->bytes_processed + c->bytes_expected){
            //读取buffer中强制类型转换
            /**
             * type Header struct {
                     Id       uint16
                     Version  uint16
                     LogId    uint32
                     Provider [16]byte
                     MagicNum uint32
                     Reserved uint32
                     BodyLen  uint32
                }
             * 信令服务中头的定义
             * */
            ghead_t *head = (ghead_t *)(c->querybuf);
            if(TcpConnection::STATE_HEAD == c->current_state){
               if(GHEAD_MAGIC_NUM != head->magic_num){
                    RTC_LOG(LS_WARNING) << "invalid data, fd: "<< c->fd << " , magic num: "<< head->magic_num;
                    return -1;
               }
               //读取到头部后 读取body的信息，body长度在head中
               c->current_state = TcpConnection::STATE_BODY;
               c->bytes_expected = head->body_len;
               c->bytes_processed = GHEAD_SIZE;
            }else {
                rtc::Slice header(c->querybuf,GHEAD_SIZE);
                rtc::Slice body(c->querybuf + GHEAD_SIZE,head->body_len);
                int ret = _process_request(c,header,body);
                if(ret != 0){
                    return -1;
                }
                //此TCP连接是短链接情况，接收到header 和 body中的内容后不在处理后续报文
                //总数据包长度绝对不会达到此常量
                c->bytes_processed = 65535;
            }
        }
        return 0;
    }

    void SignalingWorker::_close_conn(TcpConnection* c){
        RTC_LOG(LS_INFO) << "close connection, fd: "<<c->fd;
        close(c->fd);
        _remove_conn(c);
    }

    void SignalingWorker::_remove_conn(TcpConnection* c){
        _el->delete_timer(c->timer_watcher);
        _el->delete_io_event(c->io_watcher);
        _conns[c->fd] = nullptr;
        delete c;
    }

    int SignalingWorker::_process_request(TcpConnection *c, const rtc::Slice& header,const rtc::Slice& body){
        RTC_LOG(LS_INFO) << "receive body: " << body.data();
        return 0;
    }

    void conn_io_cb (EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int events,void* data){
        SignalingWorker* worker = (SignalingWorker*)data;
        if(events & EventLoop::READ){
            worker->_read_query(fd);
            return;
        }

    }
    //定时器回调
    void conn_timeout_cb(EventLoop* el,TimerWatcher* /*w*/,void* data){
        SignalingWorker* worker = (SignalingWorker*)el->owner();
        TcpConnection* c = (TcpConnection*)data;
        worker->_process_timeout(c);
    }

    void SignalingWorker::_process_timeout(TcpConnection* c){
        // RTC_LOG(LS_INFO) << "connection timeout , fd: "<<c->fd;
        if(_el->now() - c->last_interaction >= (unsigned long) _options.connection_timeout){
            RTC_LOG(LS_INFO) << "connection timeout , fd: "<<c->fd;
            _close_conn(c);
        }
    }

    void SignalingWorker::_new_conn(int fd){
        RTC_LOG(LS_INFO) << "signaling server worker_id : "<< _worker_id<<" conn fd: "<<fd;
        if(fd < 0){
            RTC_LOG(LS_WARNING) << "invalid fd:  " << fd;
            return;
        }
        //设置非阻塞
        sock_setnonblock(fd);
        ///设置
        sock_setnodelay(fd);
        TcpConnection* tcpconn = new TcpConnection(fd);
        sock_peer_2_str(fd,tcpconn->ip,&(tcpconn->port));
        //启动事件监听
        tcpconn->io_watcher = _el->create_io_event(conn_io_cb,this);
        _el->strart_io_event(tcpconn->io_watcher,fd,EventLoop::READ);
        //启动定时器 
        tcpconn->timer_watcher = _el->create_timer(conn_timeout_cb,tcpconn,true);
        _el->start_timer(tcpconn->timer_watcher,100000);//100ms
        //设置当前时间戳
        tcpconn->last_interaction = _el->now();
        if((size_t)fd >= _conns.size()){
            _conns.resize(fd * 2, nullptr);
        }
        _conns[fd] = tcpconn;


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
        RTC_LOG(LS_INFO)<<"signaling worker free resource done. "; 
    }

    void SignalingWorker::stop(){
        notify(SignalingWorker::QUIT);
    }

    void SignalingWorker::join(){
        if(_thread && _thread->joinable()){
            _thread->join();
        }
    }

   int SignalingWorker::notify_new_conn(int fd){
        _q_conn.produce(fd);
        return notify(SignalingWorker::NEW_CONN);
   } 

} // namespace grtc
