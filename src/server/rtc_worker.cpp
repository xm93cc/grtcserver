#include "server/rtc_worker.h"
#include <rtc_base/logging.h>
#include <unistd.h>
#include "server/signaling_worker.h"
namespace grtc
{
   RtcWorker::RtcWorker(int worker_id, const RtcServerOptions &options)
       : _worker_id(worker_id), _el(new EventLoop(this)),_options(options), _rtc_stream_mgr(new RtcStreamManager(_el)) {}
   RtcWorker::~RtcWorker() {
      if(_el){
         delete _el;
         _el = nullptr;
      }
      if(_thread){
         delete _thread;
         _thread = nullptr;
      }
   }
   void rtc_worker_recv_notify(EventLoop * /*el*/, IOWatcher * /*w*/, int fd, int /*events*/, void *data)
   {
      int msg;
      if (read(fd, &msg, sizeof(int)) != sizeof(int))
      {
         RTC_LOG(LS_WARNING) << "read error errno: " << errno << " error: " << strerror(errno);
         return;
      }
      RtcWorker *worker = (RtcWorker *)data;
      worker->_process_notify(msg);
   }
   int RtcWorker::init()
   {
      int fds[2];
      if (pipe(fds))
      {
         RTC_LOG(LS_WARNING) << "create pipe error. error: " << strerror(errno) << " errno: " << errno;
         return -1;
      }
      _notify_recv_fd = fds[0];
      _notify_send_fd = fds[1];
      _pipe_watcher = _el->create_io_event(rtc_worker_recv_notify, this);
      // 开始事件
      _el->start_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
      return 0;
   }

   bool RtcWorker::start()
   {
      if (_thread)
      {
         RTC_LOG(LS_WARNING) << "rtc worker already start.";
         return false;
      }
      _thread = new std::thread([=](){
          RTC_LOG(LS_INFO) << "rtc worker event loop run.";
          _el->start();
          RTC_LOG(LS_INFO) << "rtc woker event loop stop."; 
      });
          return true;
   }

   int RtcWorker::notify(int msg)
   {
      int ret = write(_notify_send_fd, &msg, sizeof(msg));
      return ret == sizeof(int) ? 0 : -1;
   }

   void RtcWorker::stop()
   {
      notify(RtcWorker::QUIT);
   }
   void RtcWorker::join()
   {
      if (_thread && _thread->joinable())
      {
         _thread->join();
      }
   }

   //处理通知
   void RtcWorker::_process_notify(int msg){
       switch (msg)
        {
        case RtcWorker::QUIT:
             _stop();
            break;
        case RtcWorker::RTC_MSG:
            _process_rtc_msg();
            break;
        default:
            RTC_LOG(LS_WARNING)<< "unknown msg: "<< msg;
            break;
        }
   }
   // 关闭描述符，事件循环，线程
   void RtcWorker::_stop()
   {
      if(!_thread){
         RTC_LOG(LS_WARNING)<<"signaling server not running.";
         return ;
      }
      _el->delete_io_event(_pipe_watcher);
      _el->stop();
      close(_notify_recv_fd);
      close(_notify_send_fd);
   }
   //发送Rtc msg
   int RtcWorker::send_rtc_msg(std::shared_ptr<RtcMsg> msg){
      //将消息投递到worker的队列
      push_msg(msg);
      return notify(RtcWorker::RTC_MSG);

   }

   // 推入RTC MSG
   void RtcWorker::push_msg(std::shared_ptr<RtcMsg> msg)
   {
      _q_msg.produce(msg);
   }
   // 弹出RTC MSG
   bool RtcWorker::pop_msg(std::shared_ptr<RtcMsg> *msg)
   {
      return _q_msg.consume(msg);
   }
     //处理rtc消息
   void RtcWorker::_process_rtc_msg(){
         std::shared_ptr<RtcMsg> msg;
         if(!pop_msg(&msg)){
            return ;
         }
      RTC_LOG(LS_INFO) << "cmdno[" << msg->cmdno << "] uid[" << msg->uid << "] stream_name[" << 
        msg->stream_name << "] audio[" << msg->audio << "] video[" << msg->video << "] log_id["<<msg->log_id <<"] worker receive msg, worker_id: " << _worker_id;

        switch (msg->cmdno)
        {
        case CMDNO_PUSH:
            _process_push(msg);
         break;
        default:
            RTC_LOG(LS_WARNING) << "unknown cmdno: " << msg->cmdno << ", log_id" << msg->log_id;
         break;
        }

   }

   // 处理push消息
   void RtcWorker::_process_push(std::shared_ptr<RtcMsg> msg)
   {
         std::string offer;
         _rtc_stream_mgr->create_push_stream(msg->uid, msg->stream_name, msg->audio, msg->video, msg->log_id, offer);
         RTC_LOG(LS_INFO) << "offer:  " << offer;
         msg->sdp = offer;
         SignalingWorker* worker = (SignalingWorker*) msg->worker;
         if(worker){
            worker->send_rtc_msg(msg);
         }
   }

} // namespace grtc
