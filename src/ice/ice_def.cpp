// impl ice_def.cpp
#include "ice/ice_def.h"
namespace grtc {
const int ICE_UFRAG_LENGTH = 4;
const int ICE_PWD_LENGTH = 24;

const int STUN_PACKET_SIZE = 60 * 8;
const int WEAK_PING_INTERVAL = 1000 * STUN_PACKET_SIZE / 10000;   // 48ms
const int STRONG_PING_INTERVAL = 1000 * STUN_PACKET_SIZE / 1000;  // 480ms
const int MIN_PINGS_AT_WEAK_PING_INTERVAL = 3;
const int STABLING_CONNECTION_PING_INTERVAL = 900;  // 900ms
const int STABLE_CONNECTION_PING_INTERVAL = 2500;   // 2500ms
const int WEAK_CONNECTION_RECEIVE_TIMEOUT = 2500;   // 2500ms
const int CONNECTION_WRITE_CONNECT_FAILS = 5 ;      // count 5
const int CONNECTION_WRITE_CONNECT_TIMEOUT = 5000;  // 5000ms
const int CONNECTION_WRITE_TIMEOUT = 15000;         //15000ms
}  // namespace grtc
