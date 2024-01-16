
#include <rtc_base/logging.h>
#include "pc/dtls_transport.h"

namespace grtc {

const size_t k_dtls_record_header_len = 13;

DtlsTransport::DtlsTransport(IceTransportChannel* ice_channel)
    : _ice_channel(ice_channel) {
        _ice_channel->signal_read_packet.connect(this, &DtlsTransport::_on_read_packet);
    }
DtlsTransport::~DtlsTransport() {
    //temp none
}

bool DtlsTransport::is_dtls_packet(const char* buf, size_t len) {
  const uint8_t* u = reinterpret_cast<const uint8_t*>(buf);
  return len >= k_dtls_record_header_len && (u[0] > 19 && u[0] < 64);
}

bool DtlsTransport::is_dtls_client_hello_packet(const char* buf, size_t len) {
  if (!is_dtls_packet(buf, len)) {
    return false;
  }

  const uint8_t* u = reinterpret_cast<const uint8_t*>(buf);
  return len > 17 && (u[0] == 22 && u[13] < 1);
}

void DtlsTransport::_on_read_packet(IceTransportChannel* /*channel*/,
                                    const char* buf, size_t len,
                                    int64_t /*ts*/) {
  switch (_dtls_state) {
    case DtlsTransportState::k_new:
      if (_dtls) {
        RTC_LOG(LS_INFO) << to_string()
                         << ": Received packet before DTLS started. ";
      } else {
        RTC_LOG(LS_INFO) << to_string()
                         << ": Received packet before we know if "
                         << "we are doing DTLS or not ";
      }
      if (is_dtls_client_hello_packet(buf, len)) {
        RTC_LOG(LS_INFO) << to_string()
                         << ": Caching DTLS ClientHello packet until "
                         << "DTLS started";
        _cached_client_hello.SetData(buf, len);
        if(!_dtls && _local_certificate){
            _setup_dtls();
        }
      } else {
        RTC_LOG(LS_INFO) << to_string() << ": Not a DTLS ClientHello packet, "
                         << "dropping";
      }
      break;

    default:
      break;
  }
}

std::string DtlsTransport::to_string() {
  std::stringstream ss;
  absl::string_view RECEIVING[2] = {"-", "R"};
  absl::string_view WRITABLE[2] = {"-", "R"};
  ss << "DtlsTransport[" << transport_name() << "|" << (int)component() << "|"
     << RECEIVING[_receiving] << "|" << WRITABLE[_writable] << "]";
  return ss.str();
}

bool DtlsTransport::_setup_dtls() {
    return false;
}

const std::string& DtlsTransport::transport_name(){
  return _ice_channel->transport_name();
}

}  // namespace grtc
