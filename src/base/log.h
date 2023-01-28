#ifndef __BASE_LOG_H_
#define __BASE_LOG_H_
#include <rtc_base/logging.h>
#include <fstream>
#include<queue>
#include<mutex>
#include<thread>
namespace grtc
{
    class GrtcLog : public rtc::LogSink
    {
        
    public:
        GrtcLog(const std::string log_dir,
                const std::string log_name,
                const std::string log_level);
        ~GrtcLog() override;
        int init();
        bool start();
        void stop();
        void join();
        void set_log_to_stderr(bool on);
        void OnLogMessage(const std::string& message, rtc::LoggingSeverity severity) override;
        void OnLogMessage(const std::string& message) override;

    private:
        // defin log base info 
        std::string _log_dir;
        std::string _log_name;
        std::string _log_level;

        //defin log file path and output stream
        std::string _log_file;
        std::string _log_file_wf;
        std::ofstream _out_file;
        std::ofstream _out_file_wf;

        //def  log queue and mutex lock up
        std::queue<std::string> _log_queue;
        std::mutex _mtx;
        std::queue<std::string> _log_queue_wf;
        std::mutex _mtx_wf;
        std::thread* _thread = nullptr;
        std::atomic<bool> _running{false};
    };

}

#endif