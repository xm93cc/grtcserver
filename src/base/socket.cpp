//impl socket.h

#include "base/socket.h"
#include<unistd.h>
#include<rtc_base/logging.h>
#include<arpa/inet.h>

namespace grtc{
    int create_tcp_server(const char* addr,int port){
        int sock = socket(AF_INET,SOCK_STREAM,0);
        if(-1 == sock){
            RTC_LOG(LS_WARNING)<<"create socket error , errno:"<<errno<<" error:"<<strerror(errno);
            return -1;
        }
        int on = 1;
        int ret = setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
        if(-1 == ret){
            RTC_LOG(LS_WARNING)<<"set socketopt SO_REUSEADDR error , errno:"<<errno<<" error:"<<strerror(errno);
            return -1;
        }

        struct sockaddr_in sa;
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);

        if(addr &&  inet_aton(addr,&sa.sin_addr) == 0){
            RTC_LOG(LS_WARNING)<<"invalid error, errno:"<<errno<<" error:"<<strerror(errno);
            return -1;
        }

        ret = bind(sock,(sockaddr *)&sa,sizeof(sa));
         if(-1 == ret){
            RTC_LOG(LS_WARNING)<<"bind error , errno:"<<errno<<" error:"<<strerror(errno);
            return -1;
        }

        //max 4096  始终有一个是使用的，用来监听所以只能使用4095
        ret = listen(sock,4095);

         if(-1 == ret){
            RTC_LOG(LS_WARNING)<<"listen error , errno:"<<errno<<" error:"<<strerror(errno);
            return -1;
        }

        return sock;   
        
    }

    int generic_accept(int sock, struct sockaddr* addr,socklen_t* len){
        int fd;
        while (1)
        {
            fd = accept(sock,addr,len);
            if(-1 == fd){
                if(EINTR == errno){
                    continue;
                }else{
                    RTC_LOG(LS_WARNING) << "tcp accept error : "<< strerror(errno) << " errno: "<< errno;
                    return -1;
                }
            }
            break;    
        }
        return fd;
    }

    int tcp_accept(int sock, char* host , int* port){
        struct sockaddr_in sa;
        socklen_t socklen = sizeof(sa);
        int fd = generic_accept(sock,(struct sockaddr*)&sa,&socklen);

        if(host){
            strcpy(host,inet_ntoa(sa.sin_addr));
        }

        if(port){
            *port = ntohs(sa.sin_port);
        }
        return fd;
    }

} // namespace grtc