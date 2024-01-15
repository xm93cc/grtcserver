#ifndef __DTLS__TRANSPORT_H_
#define __DTLS__TRANSPORT_H_
#include "ice/ice_transport_channel.h"

namespace grtc
{
class DtlsTransport : public sigslot::has_slots<>{
public:
    DtlsTransport(IceTransportChannel* ice_channel);

    ~DtlsTransport();

    const std::string& transport_name();

    IceCandidateComponent component() {return _ice_channel->component();}

private:
    void _on_read_packet(IceTransportChannel* channel, const char* buf, size_t len, int64_t ts);
private:
    IceTransportChannel* _ice_channel;   
};
} // namespace grtc

#endif //__DTLS_TRANSPORT_H_