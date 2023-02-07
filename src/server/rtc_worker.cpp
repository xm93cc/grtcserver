#include "server/rtc_worker.h"
#include <rtc_base/logging.h>
#include <unistd.h>
namespace grtc
{
   RtcWorker::RtcWorker(int worker_id, const RtcServerOptions &options)
       : _worker_id(worker_id), _el(new EventLoop(this)),_options(options) {}
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
      _el->strart_io_event(_pipe_watcher, _notify_recv_fd, EventLoop::READ);
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
        case RtcServer::QUIT:
             _stop();
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


} // namespace grtc
