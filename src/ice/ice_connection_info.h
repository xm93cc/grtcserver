#ifndef __ICE_CONNECTION_INFO_H_
#define __ICE_CONNECTION_INFO_H_

namespace grtc {
enum class IceCandidatePairState {
  WAITING,      //尚未开始连通性检查
  IN_PROGRESS,  //检查中
  SUCCEEDED,    //检查成功
  FAILED,       //检查失败
};
}  // namespace grtc

#endif  //__ICE_CONNECTION_H_