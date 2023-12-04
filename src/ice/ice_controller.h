

#ifndef __ICE_CONTROLLER_H_
#define __ICE_CONTROLLER_H_
#include "ice/ice_connection.h"
namespace grtc {
class IceTransportChannel;
struct PingResult {
  PingResult(const IceConnection* conn, int ping_interval)
      : conn(conn), ping_interval(ping_interval) {}
  const IceConnection* conn = nullptr;
  int ping_interval = 0;
};

class IceController {
 public:
  IceController(IceTransportChannel* ice_channel) : _ice_channel(ice_channel) {}

  ~IceController() = default;

  bool has_pingable_connection();

  void add_connection(IceConnection* conn);

  const std::vector<IceConnection*> connectios(){return _connections;}

  PingResult selected_connection_to_ping(int64_t last_ping_sent_ms);

 private:
  bool _weak() {
    return _selected_connection == nullptr || _selected_connection->weak();
  }

 private:
  bool _is_pingable(IceConnection* conn);

  const IceConnection* _find_next_pingable_connection(int64_t now_ms);

  bool _is_connection_past_ping_interval(const IceConnection* conn, int64_t now_ms);

  int _get_connection_ping_interval(const IceConnection* conn , int64_t now_ms);

 private:
  IceTransportChannel* _ice_channel;
  IceConnection* _selected_connection = nullptr;
  std::vector<IceConnection*> _connections;
};
}  // namespace grtc

#endif  // __ICE_CONTROLLER_H_
