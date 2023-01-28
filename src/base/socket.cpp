//impl socket.h

#include "base/socket.h"
#include<unistd.h>
#include<rtc_base/logging.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<netinet/tcp.h>

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

    int sock_setnodelay(int sock){
        int on = 1;
        if(-1 == setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&on,sizeof(on))){
            RTC_LOG(LS_WARNING)<<"setsockopt TCP_NODELAY error: "<< strerror(errno) << " errno: "<<errno << " fd: "<<sock;
            return -1;
        }
        return 0;
    }

    int sock_setnonblock(int sock){
        int flags = fcntl(sock,F_GETFL);
        if(-1 == flags){
            RTC_LOG(LS_WARNING)<<"fcntl(F_GETFL) error: "<< strerror(errno) << " errno: "<<errno << " fd: "<<sock;
            return -1;
        }

        if(-1 == fcntl(sock,F_SETFL,flags | O_NONBLOCK)){
            RTC_LOG(LS_WARNING)<<"fcntl(F_SETFL) error: "<< strerror(errno) << " errno: "<<errno << " fd: "<<sock;
            return -1;
        }
        return 0;    
    }

   int sock_peer_2_str(int fd,char* host,int* port){
        struct sockaddr_in sa;
        socklen_t socklen = sizeof(sa);
        int ret = getpeername(fd,(struct sockaddr*)&sa,&socklen);
        if(-1 == ret){
            if(host){
                host[0] = '?';
                host[1] = '\0';
            }
            if(port){
                *port = 0;
            }
            return -1;
        }

        if(host){
            strcpy(host,inet_ntoa(sa.sin_addr));
        }
        if(port){
            *port = ntohs(sa.sin_port);
        }
        return 0;
   } 

} // namespace grtc