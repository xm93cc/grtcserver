// impl ice_def.cpp
#include "ice/ice_def.h"
namespace grtc {
const int ICE_UFRAG_LENGTH = 4;
const int ICE_PWD_LENGTH = 24;

const int STUN_PACKET_SIZE = 60 * 8;
const int WEAK_PING_INTERVAL = 1000 * STUN_PACKET_SIZE / 10000;   // 48ms
const int STRONG_PING_INTERVAL = 1000 * STUN_PACKET_SIZE / 1000;  // 480ms
}  // namespace grtc
