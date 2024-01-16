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

private:
    IceTransportChannel* _ice_channel;   
    DtlsTransportState _dtls_state = DtlsTransportState::k_new;
    bool _receiving = false;
    bool _writable = false;
    std::unique_ptr<rtc::SSLStreamAdapter> _dtls;
    rtc::Buffer _cached_client_hello;
    rtc::RTCCertificate* _local_certificate = nullptr;
};
} // namespace grtc

#endif //__DTLS_TRANSPORT_H_