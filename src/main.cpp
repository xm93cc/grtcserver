#include<iostream>
#include "base/conf.h"
#include "base/log.h"
#include "server/signaling_server.h"
#include<unistd.h>
#include<signal.h>
#include"server/rtc_server.h"
//基本配置 读取设置
grtc::GeneralConf* g_conf = nullptr;
//定义Log处理(分级别异步写入文件系统)
grtc::GrtcLog* g_log = nullptr;
//定义信令处理服务
grtc::SignalingServer* g_signaling_server = nullptr;
//定义rtc server
grtc::RtcServer* g_rtc_server = nullptr;

//初始化，加载YAML配置
int init_general_conf(const char* filename){
    if(!filename){
        fprintf(stderr,"file name is nullptr\n");
        return -1;
    }
    g_conf = new grtc::GeneralConf();
    int ret = grtc::load_general_conf(filename,g_conf);
    if(ret != 0){
        fprintf(stderr,"load %s config file failed \n",filename);
        return -1;
    }
    return 0;
}
//初始化Log处理器
int init_log(const std::string& log_dir,const std::string& log_name,const std::string& log_level){
    g_log = new grtc::GrtcLog(log_dir,log_name,log_level);
    int ret = g_log->init();
    if (ret != 0){
        return -1;
    }
    return 0;
}
//通过YAML配置 初始化信令处理服务 
int init_signaling_server(const char* conf_path){
    g_signaling_server = new grtc::SignalingServer();
    int ret = g_signaling_server->init(conf_path);
    if(ret != 0){
     return ret;
    }
    return 0;
}
//通过YAML配置 初始化RTC服务 
int init_rtc_server(const char* conf_path){
    g_rtc_server = new grtc::RtcServer();
    int ret = g_rtc_server->init(conf_path);
    if(ret != 0){
        return -1;
    }
    return 0;
}
//处理信号
static void process_signal(int sig){
    RTC_LOG(LS_INFO) << "receive signal: " << sig;
    if(SIGINT == sig || SIGTERM == sig){
        if(g_signaling_server){
            g_signaling_server->stop();
        } 
        if(g_rtc_server){
            g_rtc_server->stop();
        }
    }

}
int main(){
    int ret = init_general_conf("./conf/general.yaml");
    if(ret != 0){
        return -1;
    }
    ret = init_log(g_conf->log_dir,g_conf->log_name,g_conf->log_level);
    g_log->start();
    if (ret != 0)
    {
        return -1;
    }
    g_log->set_log_to_stderr(g_conf->log_to_stderr);
    //初始化signaling server
    ret = init_signaling_server("./conf/signaling_server.yaml");
    if (ret != 0)
    {
        return -1;
    }
    //初始化rtc server
    ret = init_rtc_server("./conf/rtc_server.yaml");
    if (ret != 0)
    {
        return -1;
    }
    //捕获信号处理，CTRL+C SIGINT 和 任何kill 发出的信号
    signal(SIGINT,process_signal);
    signal(SIGTERM,process_signal);
    g_signaling_server->start();
    g_rtc_server->start();

    g_signaling_server->join();
    g_rtc_server->join();
    return 0;
}
