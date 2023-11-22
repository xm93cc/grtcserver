#ifndef __BASE_SOCKET_H_
#define __BASE_SOCKET_H_
#include<sys/socket.h>

namespace grtc{
    int create_tcp_server(const char* host,int port);
    int tcp_accept(int sock, char* host , int* port);
    //设置sock为非阻塞
    int sock_setnonblock(int sock);
    //设置sock nodelay 属性
    int sock_setnodelay(int sock);
    //读取对端的host和port信息，返回-1为异常情况
    int sock_peer_2_str(int fd,char* host,int* port);
    //从fd（sock）中读取指定长度的字节数据，返回实际读取到字节数，-1为异常情况
    int sock_read_data(int sock,char* buf,size_t len);
    //向连接中写入数据
    int sock_write_data(int sock,const char* buf,size_t len);
    /*
     * @Author: XM93CC
     * @Date: 2023-05-08 15:23:58
     * @Description: 创建UDP socket 文件描述符
     */
    int create_udp_socket(int family);

    /*
     * @Author: XM93CC
     * @Date: 2023-05-08 15:45:24
     * @Description: 绑定socket端口 如果min_port和max_port 都为0时 系统分配一个可用的端口
     *               如果min_port 和 max_port 值有效 根据这两个参数分配一个有效的端口
     */     

    int sock_bind(int sock, struct sockaddr* addr, socklen_t len, int min_port, int max_port);
    /*
     * @Author: XM93CC
     * @Date: 2023-05-08 16:02:08
     * @Description: 通过socket获取地址信息
     */
    int sock_get_address(int sock, char* ip, int* port);

    /*
     * @Author: XM93CC
     * @Date: 2023-05-31 11:21:06
     * @Description: 从sock中读取数据
     */
    int sock_recv_from(int sock, char* buf, size_t size, struct sockaddr* addr, socklen_t addr_len);

    /*
     * @Author: XM93CC
     * @Date: 2023-06-13 15:28:10
     * @Description: 读取socket接收数据包时间
     */
    int64_t sock_get_recv_timestamp(int sock);
    
    int sock_send_to(int sock, const char* buf,  size_t len, int flag, struct sockaddr* addr, socklen_t addr_len);
}//namespace grtc

#endif //__BASE_SOCKET_H_