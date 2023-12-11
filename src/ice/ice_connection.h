#ifndef __ICE_CONNECTION__
#define __ICE_CONNECTION__
#include "base/event_loop.h"
#include "ice/candidate.h"
#include "ice/stun.h"
#include "ice/stun_request.h"
#include "ice/ice_credentials.h"
namespace grtc
{
class UDPPort;
class IceConnection;
class ConnectionRequest : public StunRequest{
public:
    ConnectionRequest(IceConnection* conn);

protected:
    void prepare(StunMessage* msg) override;
private:
    IceConnection* _connection;

};

class IceConnection : public sigslot::has_slots<>{
public:
    enum WriteState{
        STATE_WRITABLE = 0,
        STATE_WRITE_UNRELIABLE = 1,
        STATE_WRITE_INIT = 2,
        STATE_WRITE_TIMEOUT = 3
    };

    struct SentPing {
      SentPing(const std::string& id, int64_t ts) : id(id), sent_time(ts) {}
      std::string id;
      int64_t sent_time;
    };

    IceConnection(EventLoop* el, UDPPort* port, const Candidate& remote_candidate);

    ~IceConnection();

    const Candidate& remote_candidate() const { return _remote_candidate; }
    
    const Candidate& local_candidate() const;

    void on_read_packet(const char* buf, size_t size, uint64_t ts);

    void handle_stun_binding_request(StunMessage* stun_msg);

    void send_stun_binding_response(StunMessage* stun_msg);

    void send_response_message(const StunMessage& resp);

    bool writable() {return _write_state == STATE_WRITABLE;}

    bool receving() {return _receiving;}

    bool weak() {return !(writable() && receving());}

    bool active() {return _write_state != STATE_WRITE_TIMEOUT;}

    void maybe_set_remote_ice_params(const IceParameters& params); 

    bool stable (int64_t now) const;

    int64_t last_ping_sent() const {return _last_ping_sent; }

    int num_pings_sent() const {return _num_pings_sent;}

    void ping(int64_t now_ms);

    UDPPort* port() { return _port; }

    std::string to_string();

private:
    void _on_stun_send_packet(StunRequest* request, const char* data, size_t len);
private:
    EventLoop* _el;
    UDPPort* _port;
    Candidate _remote_candidate;
    WriteState _write_state = STATE_WRITE_INIT;
    bool _receiving = false;
    int64_t _last_ping_sent = 0;
    int _num_pings_sent = 0;
    std::vector<SentPing> _pings_since_last_response;
    StunRequestManager _requests;
};
} // namespace grtc

#endif
