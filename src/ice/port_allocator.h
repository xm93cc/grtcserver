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
private:
    std::unique_ptr<NetworkManager> _network_manager;
};
} // namespace grtc


#endif //__PORT_ALLOCATOR_H_
