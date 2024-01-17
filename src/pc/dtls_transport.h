#ifndef __DTLS__TRANSPORT_H_
#define __DTLS__TRANSPORT_H_

#include <memory>
#include <rtc_base/ssl_stream_adapter.h>
#include <rtc_base/buffer_queue.h>
#include <rtc_base/rtc_certificate.h>
#include "ice/ice_transport_channel.h"

namespace grtc
{

enum class DtlsTransportState{
    k_new,
    k_connecting,
    k_connected,
    k_closed,
    k_failed,
    k_num_values
};

class StreamInterfaceChannel : public rtc::StreamInterface {
 public:
  StreamInterfaceChannel(IceTransportChannel* ice_channel);

  rtc::StreamState GetState() const override;

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
  rtc::StreamResult Read(void* buffer, size_t buffer_len, size_t* read,
                         int* error) override;

  rtc::StreamResult Write(const void* data, size_t data_len, size_t* written,
                          int* error) override;
  // Attempt to transition to the SS_CLOSED state.  SE_CLOSE will not be
  // signalled as a result of this call.
  void Close() override;

 private:
  IceTransportChannel* _ice_channel;
};

class DtlsTransport : public sigslot::has_slots<>{
public:
    DtlsTransport(IceTransportChannel* ice_channel);

    ~DtlsTransport();

    const std::string& transport_name();

    IceCandidateComponent component() {return _ice_channel->component();}
    
    std::string to_string();

    bool is_dtls_client_hello_packet(const char* buf, size_t len);

    bool is_dtls_packet(const char* buf, size_t len);

private:
    void _on_read_packet(IceTransportChannel* channel, const char* buf, size_t len, int64_t ts);

    bool _setup_dtls();

    bool _maybe_start_dtls();

private:
    IceTransportChannel* _ice_channel;   
    DtlsTransportState _dtls_state = DtlsTransportState::k_new;
    bool _receiving = false;
    bool _writable = false;
    std::unique_ptr<rtc::SSLStreamAdapter> _dtls;
    rtc::Buffer _cached_client_hello;
    rtc::RTCCertificate* _local_certificate = nullptr;
    StreamInterfaceChannel* _downward = nullptr;
    std::string _remote_fingerprint_alg;
    rtc::Buffer _remote_fingerprint_value;

};
} // namespace grtc

#endif //__DTLS_TRANSPORT_H_