// impl SignalingWorker.h
#include"server/signaling_worker.h"
#include<rtc_base/logging.h>
#include<unistd.h>
#include"base/socket.h"
#include"server/tcp_connection.h"
#include"grtcserver_def.h"
#include"server/rtc_server.h"
#include<rtc_base/zmalloc.h>
extern grtc::RtcServer* g_rtc_server;
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
        _el->start_io_event(_pipe_watcher,_notify_recv_fd,EventLoop::READ);
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
     //响应服务端offer
    void SignalingWorker::_response_server_offer(std::shared_ptr<RtcMsg> msg){
        TcpConnection *c = (TcpConnection *)(msg->conn);

        if (!c)
        {
           return;
        }

        // 透传TcpConnection 主要是避免在高并发时，当前操作的描述符被新的连接所覆盖 将旧的数据写到新的描述符
        // msg中透传文件描述符主要判定tcp连接超时被释放问题，指针操作被释放的内存可能出现问题
        int fd = msg->fd;
        if (fd <= 0 || size_t(fd) >= _conns.size()){
           return;
        }

        if (_conns[fd] != c){
           return;
        }

        ghead_t *xh = (ghead_t *)(c->querybuf);
        rtc::Slice header(c->querybuf, GHEAD_SIZE);
        char *buf = (char *)zmalloc(GHEAD_SIZE + MAX_RES_BUF);
        if (!buf){
           RTC_LOG(LS_WARNING) << "zmalloc error, log_id: " << xh->log_id;
           return;
        }
        memcpy(buf,header.data(),GHEAD_SIZE);
        ghead_t* res_xh = (ghead_t*)buf;
        Json::Value res_root;
        res_root["error_no"] = msg->err_no;
        if(msg->err_no != 0){
            res_root["err_msg"] = "process error";
            res_root["offer"] = "";
        }else{
            res_root["err_msg"] = "success";
            res_root["offer"] = msg->sdp;
        }
        Json::StreamWriterBuilder write_builder;
        write_builder.settings_["indentation"] = "";
        std::string json_data = Json::writeString(write_builder, res_root);
        RTC_LOG(LS_INFO) << "response body: " << json_data;
        res_xh->body_len = json_data.size();
        snprintf(buf + GHEAD_SIZE, MAX_RES_BUF, "%s", json_data.c_str());
        rtc::Slice reply(buf, GHEAD_SIZE + res_xh->body_len);//组合数据包结构
        _add_reply(c,reply);


    }

    void SignalingWorker::_add_reply(TcpConnection* c, const rtc::Slice& reply){
        c->reply_list.push_back(reply);
        _el->start_io_event(c->io_watcher, c->fd, EventLoop::WRITE);
    }

    //处理队列中的RtcMsg
    void SignalingWorker::_process_rtc_msg(){
        std::shared_ptr<RtcMsg> msg = pop_msg();
        if(!msg){
            return;
        }
        switch (msg->cmdno)
        {
        case CMDNO_PUSH:
            _response_server_offer(msg);
            break;
        
        default:
            RTC_LOG(LS_WARNING) << "unknown cmdno: " << msg->cmdno << ", log_id: " << msg->log_id;
            break;
        }
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
        case SignalingWorker::RTC_MSG:
            _process_rtc_msg();
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
        ghead_t* gh = (ghead_t*)(header.data());
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        Json::Value root;
        JSONCPP_STRING err;
        reader->parse(body.data(),body.data()+body.size(),&root,&err);
        if(!err.empty()){
            RTC_LOG(LS_WARNING) << "parse json body error: " << err << " ,fd: "<< c->fd << " ,log_id: "<< gh->log_id ;
            return -1;
        }
        int cmdno;
        try{
            cmdno = root["cmdno"].asInt();
        }
        catch(const Json::Exception& e){      
            RTC_LOG(LS_WARNING) << "no cmdno field in body, log_id: "<< gh->log_id; 
            return -1;
        }
        int ret = 0;
        switch (cmdno)
        {
        case CMDNO_PUSH:
            return _process_push(cmdno,c,root,gh->log_id);
        case CMDNO_ANSWER:
            ret = _process_answer(cmdno, c, root, gh->log_id);
            break;
        default:
            RTC_LOG(LS_WARNING) << "unknown cmdno: " << cmdno << ", log_id: " << gh->log_id;
            break;
        }

        //return result

        char* buf = (char*) zmalloc(GHEAD_SIZE + MAX_RES_BUF);
        ghead_t* res_gh = (ghead_t*) buf;
        memcpy(res_gh, header.data(), header.size());
        Json::Value res_root;
        if (0 == ret)
        {
            res_root["err_no"] = ret;
            res_root["err_msg"] = "success";
        }else{
            res_root["err_no"] = ret;
            res_root["err_msg"] = "process error";
        }

        Json::StreamWriterBuilder write_builder;
        write_builder.settings_["indentation"] = "";
        std::string json_data = Json::writeString(write_builder, res_root);
        RTC_LOG(LS_INFO) << "response bodey: " << json_data;
        
        res_gh->body_len  = json_data.size();
        snprintf(buf + GHEAD_SIZE, MAX_RES_BUF, "%s", json_data.c_str());
        rtc::Slice reply(buf, GHEAD_SIZE + res_gh->body_len);
        _add_reply(c, reply);
        return 0;
    }

    int SignalingWorker::_process_answer(int cmdno, TcpConnection * /*c*/, const Json::Value &root, uint32_t log_id)
    {
        uint64_t uid;
        std::string stream_name;
        std::string answer;
        std::string stream_type;
        try
        {
            uid = root["uid"].asUInt64();
            stream_name = root["stream_name"].asString();
            answer = root["answer"].asString();
            stream_type = root["type"].asString();
        }
        catch (const Json::Exception &e)
        {
            RTC_LOG(LS_WARNING) << "parse json body error: " << e.what() << ", log_id: " << log_id;
            return -1;
        }
        RTC_LOG(LS_INFO) << "cmdno[" << cmdno << "] uid[" << uid << "] stream_name[" << stream_name 
                         << "] answer[" << answer << "] stream_type[" << stream_type << "] signaling server send answer request";
        std::shared_ptr<RtcMsg> msg = std::make_shared<RtcMsg>();
        msg->cmdno = cmdno;
        msg->stream_name = stream_name;
        msg->uid = uid;
        msg->sdp = answer;
        msg->stream_type = stream_type;
        msg->log_id = log_id;
        return g_rtc_server->send_rtc_msg(msg);
    }

    int SignalingWorker::_process_push(int cmdno,TcpConnection* c,const Json::Value& root,uint32_t log_id){
        uint64_t uid;
        std::string stream_name;
        int audio;
        int video;
        try{
            uid = root["uid"].asUInt64();
            stream_name = root["stream_name"].asString();
            audio = root["audio"].asInt();
            video = root["video"].asInt();
        }
        catch (const Json::Exception& e){
            RTC_LOG(LS_WARNING) << "parse json body error: " << e.what() << ", log_id: " << log_id;
            return -1;
        }
        RTC_LOG(LS_INFO) << "cmdno[" << cmdno << "] uid[" << uid << "] stream_name[" << 
        stream_name << "] audio[" << audio << "] video[" << video << "] signaling server push request";
        std::shared_ptr<RtcMsg> msg = std::make_shared<RtcMsg>();
        msg->cmdno = cmdno;
        msg->stream_name = stream_name;
        msg->uid = uid;
        msg->audio = audio;
        msg->video = video;
        msg->log_id = log_id;
        msg->worker = this;
        msg->conn = c;
        msg->fd = c->fd;
        return g_rtc_server->send_rtc_msg(msg);
    }

    void conn_io_cb (EventLoop* /*el*/,IOWatcher* /*w*/,int fd, int events,void* data){
        SignalingWorker* worker = (SignalingWorker*)data;
        if(events & EventLoop::READ){
            worker->_read_query(fd);
            return;
        }
        if(events & EventLoop::WRITE){
            worker->_write_reply(fd);
        }

    }


    void SignalingWorker::_write_reply(int fd){
        if(fd <= 0 || (size_t)fd >= _conns.size()){
            return;
        }
        TcpConnection* c= _conns[fd];
        if(!c){
            return;
        }

        while (!c->reply_list.empty())
        {
            rtc::Slice reply = c->reply_list.front();
            int nwrtten = sock_write_data(c->fd, reply.data() + c->cur_resp_pos, reply.size() - c->cur_resp_pos);
            if(-1 == nwrtten){
                _close_conn(c);
                return;
            }else if(0 == nwrtten){
                RTC_LOG(LS_WARNING) << "write zero bytes, fd: " << c->fd << " worker_id: " << _worker_id;
            }else if((nwrtten + c->cur_resp_pos) >= reply.size()){
                //写入完成
                c->reply_list.pop_front();
                zfree((void*)reply.data());
                c->cur_resp_pos = 0;
                RTC_LOG(LS_INFO) << "write finished, fd: "<< c->fd << ", worker_id: " << _worker_id;
            }else{
                c->cur_resp_pos += nwrtten;
            }
        }
        c->last_interaction = _el->now();
        if(c->reply_list.empty()){
            _el->stop_io_event(c->io_watcher, c->fd , EventLoop::WRITE);
            RTC_LOG(LS_INFO) << "stop write event, fd: "<< c->fd << ", worker_id: " << _worker_id;
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
        _el->start_io_event(tcpconn->io_watcher,fd,EventLoop::READ);
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

   // 将rtc msg 从其他历程推送到signaling worker
   int SignalingWorker::send_rtc_msg(std::shared_ptr<RtcMsg> msg)
   {
        push_msg(msg);
        return notify(SignalingWorker::RTC_MSG);
   }

   // 向队列push msg
   void SignalingWorker::push_msg(std::shared_ptr<RtcMsg> msg)
   {
        //std lock
        std::unique_lock<std::mutex> lock(_q_msg_mtx);
        _q_msg.push(msg);
   }
   // 从队列弹出 msg
   std::shared_ptr<RtcMsg> SignalingWorker::pop_msg()
   {
       std::unique_lock<std::mutex> lock(_q_msg_mtx);
       if(_q_msg.empty()){
        return nullptr;
       }
       std::shared_ptr<RtcMsg> msg = _q_msg.front();
       _q_msg.pop();
       return msg;
   }

} // namespace grtc
