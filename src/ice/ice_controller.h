

#ifndef __ICE_CONTROLLER_H_
#define __ICE_CONTROLLER_H_
#include "ice/ice_connection.h"
namespace grtc {
class IceTransportChannel;
class IceController {
 public:
  IceController(IceTransportChannel* ice_channel) : _ice_channel(ice_channel) {}

  ~IceController() = default;

  bool has_pingable_connection();

  void add_connection(IceConnection* conn);

  const std::vector<IceConnection*> connectios(){return _connections;}

 private:
  bool _weak() {
    return _selected_connection == nullptr || _selected_connection->weak();
  }

 private:
  bool _is_pingable(IceConnection* conn);

 private:
  IceTransportChannel* _ice_channel;
  IceConnection* _selected_connection = nullptr;
  std::vector<IceConnection*> _connections;
};
}  // namespace grtc

#endif  // __ICE_CONTROLLER_H_
