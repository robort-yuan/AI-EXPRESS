/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:30:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MULTIVIOPLUGIN_VIOMESSAGE_H_
#define INCLUDE_MULTIVIOPLUGIN_VIOMESSAGE_H_

#include <memory>
#include <vector>
#include <string>
#ifdef J3_MEDIA_LIB
#include "./hb_cam_interface.h"
#include "./hb_vio_interface.h"
#endif

#include "xproto_msgtype/vioplugin_data.h"
#include "horizon/vision_type/vision_type.hpp"
#include "hobot_vision/blocking_queue.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {


using hobot::vision::PymImageFrame;
using horizon::vision::xproto::basic_msgtype::VioMessage;

struct VioFeedbackContext {
#ifdef J3_MEDIA_LIB
  hb_vio_buffer_t src_info_;
  pym_buffer_t pym_img_info_;
  int pipe_id_;
#endif
};

struct ImageVioMessage : VioMessage {
 public:
  ImageVioMessage() = delete;
  explicit ImageVioMessage(
      std::vector<std::shared_ptr<PymImageFrame>> &image_frame,
      uint32_t img_num, bool is_valid = true);
  ~ImageVioMessage();

  // serialize proto
  std::string Serialize() { return "No need serialize"; }

  void FreeImage(int tmp);
  void FreeImage();
};

struct DropVioMessage : VioMessage {
 public:
  DropVioMessage() = delete;
  explicit DropVioMessage(uint64_t timestamp, uint64_t seq_id, int channel_id);
  ~DropVioMessage() {}

  // serialize proto
  std::string Serialize() override;
};

struct MultiVioMessage : VioMessage {
 public:
  MultiVioMessage() {
    LOGI << "MultiVioMessage()";
    type_ = TYPE_MULTI_IMAGE_MESSAGE;}
  std::vector<std::shared_ptr<ImageVioMessage>> multi_vio_img_;
  ~MultiVioMessage() {
    LOGI << "~MultiVioMessage";
    multi_vio_img_.clear();}
};

using MultiVioMessagePtr = std::shared_ptr<MultiVioMessage>;
}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_MULTIVIOPLUGIN_VIOMESSAGE_H_
