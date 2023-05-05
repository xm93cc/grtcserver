



#include "ice/port_allocator.h"

namespace grtc
{
/*
 * @Author: XM93CC
 * @Date: 2023-04-03 23:04:24
 * @Description: 构造NetworkManager
 */
PortAllocator::PortAllocator():_network_manager(new NetworkManager())
{
    _network_manager->create_networks();
}
PortAllocator::~PortAllocator() = default;

/*
 * @Author: XM93CC
 * @Date: 2023-04-03 23:04:58
 * @Description: 获取网络信息
 */
const std::vector<Network*>& PortAllocator::get_networks()
{
    return _network_manager->get_networks();
}
} // namespace grtc
