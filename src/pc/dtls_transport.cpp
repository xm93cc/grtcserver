
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
    auto downward = std::make_unique<StreamInterfaceChannel>(_ice_channel);
    StreamInterfaceChannel* downward_ptr = downward.get();
    _dtls = rtc::SSLStreamAdapter::Create(std::move(downward));
    if(!_dtls){
        RTC_LOG(LS_WARNING) << to_string() << ": Failed to create SSLStreamAdapter";
        return false;
    }
    _downward = downward_ptr;
    _dtls->SetIdentity(_local_certificate->identity()->Clone());
    _dtls->SetMode(rtc::SSL_MODE_DTLS);
    _dtls->SetMaxProtocolVersion(rtc::SSL_PROTOCOL_DTLS_12);
    _dtls->SetServerRole(rtc::SSL_SERVER);
    if(_remote_fingerprint_value.size() && !_dtls->SetPeerCertificateDigest(_remote_fingerprint_alg, _remote_fingerprint_value.data(),_remote_fingerprint_value.size())){
        RTC_LOG(LS_WARNING) << to_string() << ": Failed to set remote fingerprint";
        return false;
    }
    RTC_LOG(LS_INFO) << to_string() << ": Setup DTLS complete";
    _maybe_start_dtls(); 
    return true;
}

bool DtlsTransport::_maybe_start_dtls(){
    return true;
}

const std::string& DtlsTransport::transport_name(){
  return _ice_channel->transport_name();
}

StreamInterfaceChannel::StreamInterfaceChannel(IceTransportChannel* ice_channel)
    : _ice_channel(ice_channel) {}

rtc::StreamState StreamInterfaceChannel::GetState() const{}

// Read attempts to fill buffer of size buffer_len.  Write attempts to send
// data_len bytes stored in data.  The variables read and write are set only
// on SR_SUCCESS (see below).  Likewise, error is only set on SR_ERROR.
// Read and Write return a value indicating:
//  SR_ERROR: an error occurred, which is returned in a non-null error
//    argument.  Interpretation of the error requires knowledge of the
//    stream's concrete type, which limits its usefulness.
//  SR_SUCCESS: some number of bytes were successfully written, which is
//    returned in a non-null read/write argument.
//  SR_BLOCK: the stream is in non-blocking mode, and the operation would
//    block, or the stream is in SS_OPENING state.
//  SR_EOS: the end-of-stream has been reached, or the stream is in the
//    SS_CLOSED state.
rtc::StreamResult StreamInterfaceChannel::Read(void* buffer, size_t buffer_len,
                                               size_t* read, int* error) {}

rtc::StreamResult StreamInterfaceChannel::Write(const void* data,
                                                size_t data_len,
                                                size_t* written, int* error) {}
// Attempt to transition to the SS_CLOSED state.  SE_CLOSE will not be
// signalled as a result of this call.
void StreamInterfaceChannel::Close() {}

}  // namespace grtc
