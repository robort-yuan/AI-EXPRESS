/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "multivioplugin/viomessage.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_type_util.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

ImageVioMessage::ImageVioMessage(
    std::vector<std::shared_ptr<PymImageFrame>> &image_frame, uint32_t img_num,
    bool is_valid) {
  type_ = TYPE_IMAGE_MESSAGE;
  num_ = img_num;
  if (image_frame.size() > 0) {
    time_stamp_ = image_frame[0]->time_stamp;
    sequence_id_ = image_frame[0]->frame_id;
    channel_ = image_frame[0]->channel_id;
    image_.resize(image_frame.size());
  }
  is_valid_uri_ = is_valid;
  for (uint32_t i = 0; i < image_frame.size(); ++i) {
    image_[i] = image_frame[i];
  }
  LOGI << "create one ImageVioMessage, channel:" << channel_
       << ", timestamp:" << time_stamp_
       << ", frame_id:" << sequence_id_;
}

ImageVioMessage::~ImageVioMessage() {
  LOGI << "release one ImageVioMessage, channel:" << channel_
       << ", timestamp:" << time_stamp_
       << ", frame_id:" << sequence_id_;
}

void ImageVioMessage::FreeImage(int tmp) {
  if (image_.size() > 0) {
    // free image
    if (num_ == 1) {
      LOGI << "begin remove one vio slot";
#if defined(J3_MEDIA_LIB)
      VioFeedbackContext *feedback_context =
          reinterpret_cast<VioFeedbackContext *>(image_[0]->context);
      if (feedback_context == nullptr) {
          LOGE << "feedback_context pointer is NULL";
          return;
      }
      int pipe_id_ = feedback_context->pipe_id_;
      int ret = hb_vio_free_pymbuf(pipe_id_, HB_VIO_PYM_FEEDBACK_SRC_DATA,
                          &(feedback_context->src_info_));
      if (ret < 0) {
        LOGE << "hb_vio_free_pymbuf HB_VIO_PYM_FEEDBACK_SRC_DATA failed";
      }

      ret = hb_vio_free_pymbuf(pipe_id_, HB_VIO_PYM_DATA,
                               &(feedback_context->pym_img_info_));
      if (ret != 0) {
        LOGE << "hb_vio_free_pymbuf HB_VIO_PYM_DATA failed";
      }
      free(feedback_context);
      LOGD << "free feedback context success";
#endif
      image_[0]->context = nullptr;
      image_[0] = nullptr;
    } else if (num_ == 2) {
      LOGF << "should not come here";
    }
    image_.clear();
  }
}

void ImageVioMessage::FreeImage() {
  if (image_.size() > 0) {
    // free image
    if (num_ == 1) {
      LOGI << "begin remove one vio slot";
#if defined(J3_MEDIA_LIB)
      pym_buffer_t *img_info =
          reinterpret_cast<pym_buffer_t *>(image_[0]->context);
      int pipe_id = channel_;
      hb_vio_free_pymbuf(pipe_id, HB_VIO_PYM_DATA, img_info);
      free(img_info);
#endif
      image_[0]->context = nullptr;
      image_[0] = nullptr;
    } else if (num_ == 2) {
      LOGF << "should not come here";
    }
    image_.clear();
  }
}

DropVioMessage::DropVioMessage(uint64_t timestamp, uint64_t seq_id,
                               int channel_id) {
  type_ = TYPE_DROP_MESSAGE;
  time_stamp_ = timestamp;
  sequence_id_ = seq_id;
  channel_ = channel_id;
}

std::string DropVioMessage::Serialize() {
  // todo
  return "";
}



}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
