
#include <rtc_base/logging.h>
#include "pc/dtls_transport.h"

namespace grtc {
DtlsTransport::DtlsTransport(IceTransportChannel* ice_channel)
    : _ice_channel(ice_channel) {
        _ice_channel->signal_read_packet.connect(this, &DtlsTransport::_on_read_packet);
    }
DtlsTransport::~DtlsTransport() {
    //temp none
}

void DtlsTransport::_on_read_packet(IceTransportChannel* /*channel*/, const char* /*buf*/, size_t len, int64_t /*ts*/){
    RTC_LOG(LS_INFO) << "============DTLS: packet: " << len;
}


const std::string& DtlsTransport::transport_name(){
  return _ice_channel->transport_name();
}

}  // namespace grtc
