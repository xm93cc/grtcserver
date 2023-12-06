

#include "ice/stun_request.h"

namespace grtc {

void StunRequestManager::send(StunRequest* request){
    request->construct();
}

StunRequest::StunRequest(StunMessage* request) : _msg(request) {}

StunRequest::~StunRequest(){}

void StunRequest::construct(){
    prepare(_msg);
}
}  // namespace grtc
