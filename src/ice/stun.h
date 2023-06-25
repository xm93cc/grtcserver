#ifndef __ICE_STUN_H_
#define __ICE_STUN_H_
/*
 * @Author: XM93CC
 * @Date: 2023-06-19 15:37:23
 * @Description: stun message 定义
 */
#include <rtc_base/byte_buffer.h>
#include <string>
#include <memory>
#include <vector>
namespace grtc {
const size_t k_stun_header_size = 20;
const size_t k_stun_attribute_header_size = 4;
const uint32_t k_stun_magic_cookie = 0x2112A442;
const size_t k_stun_magic_cookie_length = sizeof(k_stun_magic_cookie);
const size_t k_stun_transaction_id_offset = 8;
const size_t k_stun_transaction_id_length = 12;
enum StunAttributeValue{
    STUN_ATTR_FINGERPRINT = 0x8028
};

class StunAttribute;
class StunMessage {
  private:
    uint16_t _type;
    uint16_t _length;
    std::string _transaction_id;
    std::vector<std::unique_ptr<StunAttribute>> _attrs;
  private:
    std::unique_ptr<StunAttribute> _create_attribute(uint16_t type, uint16_t length);
  public:
    static bool validate_fingerprint(const char *data, size_t len);
    StunMessage(/* args */);
    ~StunMessage();
    /*
     * 读取StunMessage
     */
    bool read(rtc::ByteBufferReader* buf);
};
class StunAttribute {
  private:
    uint16_t _type;
    uint16_t _length;
  
  public:
    virtual bool read(rtc::ByteBufferReader* buf) = 0;
    virtual ~StunAttribute();
    StunAttribute(uint16_t type, uint16_t length);
};
class StunUint32Attribute : public StunAttribute{
public:
    static const size_t SIZE = 4;
};
} // namespace grtc

#endif //__ICE_STUN_H_