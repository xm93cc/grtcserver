

#ifndef __STUN_REQUEST_
#define __STUN_REQUEST_

#include <map>
#include <rtc_base/third_party/sigslot/sigslot.h>
#include "ice/stun.h"

namespace grtc {
class StunRequest;
class StunRequestManager {
 public:
  StunRequestManager() = default;
 
  ~StunRequestManager() = default;

  void send(StunRequest* request);
 
  bool check_response(StunMessage* msg);

  sigslot::signal3<StunRequest*, const char*, size_t> signal_send_packet;

 private:
  typedef std::map<std::string, StunRequest*> RequestMap;
  RequestMap _requests;
};
class StunRequest {
 public:
  StunRequest(StunMessage* request);
  
  virtual ~StunRequest();
  
  const std::string& id() { return _msg->transaction_id(); }
  
  void construct();

  void send();

  void set_manager(StunRequestManager* manager) { _manager = manager; }

  int type() { return _msg->type(); }

  int elapsed();

 protected:
  virtual void prepare(StunMessage*) {}
  virtual void on_request_response(StunMessage*){};
  virtual void on_request_error_response(StunMessage*){};
  friend class StunRequestManager;
 private:
  StunMessage* _msg;
  StunRequestManager* _manager = nullptr;
  int64_t _ts = 0;
};
}  // namespace grtc

#endif  //__STUN_REQUEST