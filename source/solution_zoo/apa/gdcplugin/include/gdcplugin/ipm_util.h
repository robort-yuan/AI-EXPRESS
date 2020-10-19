/* @Description: ipm util decalration
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-05 17:55:25
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-05 19:33:16
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#ifndef INCLUDE_GDCPLUGIN_IPM_UTIL_H_
#define INCLUDE_GDCPLUGIN_IPM_UTIL_H_

#include <vector>
#include <string>
#include <memory>
#include "gdcplugin/stitch_image.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobotxsdk/xstream_sdk.h"

using hobot::vision::ImageFrame;
using hobot::vision::BBox;
using hobot::vision::PymImageFrame;
using hobot::vision::CVImageFrame;

namespace horizon {
namespace vision {
namespace xproto {
namespace gdcplugin {

class IpmUtil {
 public:
  IpmUtil() = default;
  ~IpmUtil() = default;
  int Init(const std::string cfg0, const std::string cfg1,
           const std::string cfg2, const std::string cfg3,
           std::vector<int> c2d, uint32_t source_num);
  int GenIPMImage(std::vector<std::shared_ptr<PymImageFrame>> pym_imgs,
                  std::vector<std::shared_ptr<CVImageFrame>> &outs);

 private:
  uint32_t source_num_;
  std::vector<int> channel2direction_;
};

}  // namespace gdcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  //  INCLUDE_GDCPLUGIN_IPM_UTIL_H_
