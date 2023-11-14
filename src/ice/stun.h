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
const size_t k_stun_message_integrity_size = 20;

enum StunMessageType{
  STUN_BINDING_REQUEST = 0x0001
};

enum StunAttributeType{
  STUN_ATTR_USERNAME = 0x0006,
  STUN_ATTR_PRIORITY = 0x0024,
  STUN_ATTR_MESSAGE_INTEGRITY = 0x0008,
  STUN_ATTR_FINGERPRINT = 0x8028,
};

enum StunAttributeValueType{
  STUN_VALUE_UNKNOWN = 0,
  STUN_VALUE_UINT32,
  STUN_VALUE_BYTE_STRING,
};

enum StunErrorCode{
  STUN_ERROR_BAD_REQUEST = 400,
  STUN_ERROR_UNATHORIZED = 401,
  STUN_ERROR_SERVER_ERROR = 500,
};

extern const char STUN_ERROR_REASON_BAD_REQUEST[];
extern const char STUN_ERROR_REASON_UNATHORIZED[];
extern const char STUN_ERROR_REASON_SERVER_ERROR[];
std::string stun_method_to_string(int type);

class StunAttribute;
class StunByteStringAttribute;
class StunUint32Attribute;
class StunMessage {

  private:
    StunAttribute* _create_attribute(uint16_t type, uint16_t length);
    const StunAttribute* _get_attribute(uint16_t type);
    bool _validate_message_integrity_of_type(uint16_t mi_attr_type, size_t mi_attr_size, const char* data, 
                                            size_t size, const std::string& password);

  public:
    enum class IntegrityStatus{
      k_not_set,
      k_no_integrity,
      k_integrity_ok,
      k_integrity_bad
    };
    static bool validate_fingerprint(const char *data, size_t len);
    StunMessage(/* args */);
    ~StunMessage();
    //读取StunMessage
    bool read(rtc::ByteBufferReader* buf);
    StunAttributeValueType get_attrribute_value_type(int type);
    const StunByteStringAttribute* get_byte_string(uint16_t type);
    const StunUint32Attribute* get_uint32(uint16_t type);
    const std::string& transaction_id()const{return _transaction_id;}
    uint16_t type()const{return _type;}
    size_t length()const{return _length;}
    StunMessage::IntegrityStatus validate_message_integrity(const std::string& password);


  private:
    uint16_t _type;
    uint16_t _length;
    std::string _transaction_id;
    std::vector<std::unique_ptr<StunAttribute>> _attrs;
    IntegrityStatus _integrity = IntegrityStatus::k_not_set;
    std::string _password;
    std::string _buffer;
};


class StunAttribute {
  private:
    uint16_t _type;
    uint16_t _length;
  
  public:
    virtual bool read(rtc::ByteBufferReader* buf) = 0;
    virtual ~StunAttribute();
    int type() const{return _type;}
    size_t length() const {return _length;}

    static StunAttribute* create(StunAttributeValueType value_type, uint16_t type, uint16_t length, void* owner);
  protected:
    StunAttribute(uint16_t type, uint16_t length);
    void consume_padding(rtc::ByteBufferReader* buf);
};


class StunUint32Attribute : public StunAttribute{
public:
    static const size_t SIZE = 4;
    uint32_t _bits;
    StunUint32Attribute(uint16_t type);
    StunUint32Attribute(uint16_t type, uint32_t value);
    ~StunUint32Attribute()override{}
    uint32_t value()const{return _bits;}
    bool read(rtc::ByteBufferReader* buf)override;

};
class StunByteStringAttribute : public StunAttribute {
  public:
    StunByteStringAttribute(uint16_t type, uint16_t length);
    ~StunByteStringAttribute() override;
    bool read(rtc::ByteBufferReader* buf) override;
    std::string get_string() const {return std::string(_bytes,length());}
  private:
    char *_bytes = nullptr;
};
} // namespace grtc

#endif //__ICE_STUN_H_