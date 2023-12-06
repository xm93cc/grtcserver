

#ifndef __STUN_REQUEST_
#define __STUN_REQUEST_

#include "ice/stun.h"

namespace grtc {
class StunRequest;
class StunRequestManager {
 public:
  StunRequestManager() = default;
 
  ~StunRequestManager() = default;

  void send(StunRequest* request);
};
class StunRequest {
 public:
  StunRequest(StunMessage* request);
  
  virtual ~StunRequest();
  
  const std::string& id() { return _msg->transaction_id(); }
  
  void construct();

 protected:
  virtual void prepare(StunMessage*){}

 private:
  StunMessage* _msg;
};
}  // namespace grtc

#endif  //__STUN_REQUEST