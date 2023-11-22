//impl socket.h

#include<sys/ioctl.h>
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

   int sock_read_data(int sock,char* buf,size_t len){
       int ret =  read(sock,buf,len);
       if(-1 == ret){
            if(EAGAIN == errno){
               ret = 0;
            }else{
                RTC_LOG(LS_WARNING) << "sock read failed, error: "<< strerror(errno) << " , errno: "<< errno << " , sock: "<<sock;
                return -1;
            }
       }else if(0 == ret){
            RTC_LOG(LS_WARNING) << "connection closed, fd: " << sock;
            return -1;
       }
       return ret;
   }

   int sock_write_data(int sock,const char* buf,size_t len){
        int ret = write(sock,buf,len);
        if(-1 == ret){
            if(EAGAIN == errno){
                return 0;
            }else{
                RTC_LOG(LS_WARNING) << "sock read failed, error: "<< strerror(errno) << " , errno: "<< errno << " , sock: "<<sock;
                return -1;
            }
        }

        return ret;
   }

    int create_udp_socket(int family)
    {
        int sock = socket(family, SOCK_DGRAM, 0);
        if (-1 == sock)
        {
            RTC_LOG(LS_WARNING) << "create udp socket error: " << strerror(errno) 
            << ", errno: " << errno;
            return -1;            
        }

        return sock;        
    }


    int sock_bind(int sock, struct sockaddr* addr, socklen_t len, int min_port, int max_port)
    {
        int ret = -1;
        if (0 == min_port || 0 == max_port)
        {
            ret = bind(sock, addr, len);
        }else{
            struct sockaddr_in* addr_in = (struct sockaddr_in*)addr;

            for(int port = min_port; port <= max_port && ret != 0 ; ++port){
                addr_in->sin_port = htons(port);
                ret = bind(sock,addr,len);
            }
        }
        if(ret != 0){
            RTC_LOG(LS_WARNING) << "bind error: " << strerror(errno) << ", erron: "<< errno ;
        }

        return ret;
        
    }


    int sock_get_address(int sock, char* ip, int* port)
    {
        struct sockaddr_in addr_in;
        socklen_t len = sizeof(sockaddr);
        int ret = getsockname(sock, (struct sockaddr*)&addr_in, &len);
        if (ret != 0)
        {
            RTC_LOG(LS_WARNING) << "getsockname error: " << strerror(errno) << ", errno: " << errno; 
            return -1;
        }

        if(ip){
            strcpy(ip,inet_ntoa(addr_in.sin_addr));
        }
        if(port){
            *port = ntohs(addr_in.sin_port);
        }
        return 0;
    }

    int sock_recv_from(int sock, char *buf, size_t len, struct sockaddr *addr,
                       socklen_t addr_len) {
        int received = recvfrom(sock, buf, len, 0, addr, &addr_len);
        if (received < 0) {
            if (EAGAIN == errno) {
                received = 0;
            } else {
                RTC_LOG(LS_WARNING) << "recv from error: " << strerror(errno)
                                    << ", erron: " << errno;
                return -1;
            }
        } else if (0 == received) {
            RTC_LOG(LS_WARNING) << "recv from error: " << strerror(errno)
                                << ", erron: " << errno;
            return -1;
        }
        return received;
    }

    int64_t sock_get_recv_timestamp(int sock){
        struct timeval time;
        int ret = ioctl(sock, SIOCGSTAMP_OLD, &time);
        if(ret != 0){
            return -1;
        }
        return time.tv_sec * 1000000 + time.tv_usec;
    }

    int sock_send_to(int sock, const char* buf, size_t len, int flag,
                     struct sockaddr* addr, socklen_t addr_len) {
      int sent = sendto(sock, buf, len, flag, addr, addr_len);
      if (sent < 0) {
        if (EAGAIN == errno) {
          sent = 0;
        } else {
          RTC_LOG(LS_WARNING)
              << "sendto error: " << strerror(errno) << ", erron: " << errno;
          return -1;
        }
      } else if (sent == 0) {
        RTC_LOG(LS_WARNING)
            << "sendto error: " << strerror(errno) << ", erron: " << errno;
        return -1;
      }
      return -1;
    }
} // namespace grtc