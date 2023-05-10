/**
 * def ice allocator 
 * 包装获取网络信息 合理的做法进程启动时去做
*/

#ifndef __PORT_ALLOCATOR_H_
#define __PORT_ALLOCATOR_H_
#include  <memory>
#include "base/network.h"
namespace grtc{
class PortAllocator{
public:
    PortAllocator();
    ~PortAllocator();
    //获取网络信息
    const std::vector<Network*>& get_networks();
    void set_port_range(int min_port, int max_port);
    int get_max_port() {return _max_port;}
    int get_min_port() {return _min_port;}
private:
    std::unique_ptr<NetworkManager> _network_manager;
    int _min_port = 0;
    int _max_port = 0;
};
} // namespace grtc


#endif //__PORT_ALLOCATOR_H_
