/*!
 * Copyright (c) 2016-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     uvcplugin.cpp
 * \Author   ronghui.zhang
 * \Version  1.0.0.0
 * \Date     2020.5.12
 * \Brief    implement of api file
 */

#include "uvcplugin/uvcplugin.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "./ring_queue.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "utils/time_helper.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XPluginErrorCode;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::basic_msgtype::VioMessage;
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_UVC_MESSAGE)
using std::chrono::milliseconds;
using hobot::Timer;

UvcPlugin::UvcPlugin(std::string config_file) {
  config_file_ = config_file;
  LOGI << "UwsPlugin smart config file:" << config_file_;
  stop_flag_ = false;
  Reset();
}

UvcPlugin::~UvcPlugin() {
  //  config_ = nullptr;
}

int UvcPlugin::ParseConfig(std::string config_file) {
  std::ifstream ifs(config_file);
  if (!ifs.is_open()) {
    LOGF << "open config file " << config_file << " failed";
    return -1;
  }
  Json::CharReaderBuilder builder;
  std::string err_json;
  Json::Value json_obj;
  try {
    bool ret = Json::parseFromStream(builder, ifs, &json_obj, &err_json);
    if (!ret) {
      LOGF << "invalid config file " << config_file;
      return -1;
    }
  } catch (std::exception &e) {
    LOGF << "exception while parse config file " << config_file << ", "
         << e.what();
    return -1;
  }
  if (json_obj.isMember("smart_transfer_mode")) {
    smart_transfer_mode_ = json_obj["smart_transfer_mode"].asUInt();
    LOGD << "smart transfer mode = " << smart_transfer_mode_;
  } else {
    LOGE << config_file << " should set smart_transfer mode";
    return -1;
  }
  return 0;
}

int UvcPlugin::Init() {
  LOGI << "UvcPlugin Init";
  RingQueue<VIDEO_STREAM_S>::Instance().Init(8, [](VIDEO_STREAM_S &elem) {
    if (elem.pstPack.vir_ptr) {
      free(elem.pstPack.vir_ptr);
      elem.pstPack.vir_ptr = nullptr;
    }
  });
  memset(&h264_sps_frame_, 0, sizeof(VIDEO_STREAM_S));
  if (0 != ParseConfig(config_file_)) {
    return -1;
  }

  uvc_server_ = std::make_shared<UvcServer>();
  if (uvc_server_->Init(config_file_)) {
    LOGE << "UwsPlugin Init uWS server failed";
    return -1;
  }
  if (smart_transfer_mode_ == 1) {
    smart_manager_ = std::make_shared<RndisManager>(config_file_);
  } else if (smart_transfer_mode_ == 0) {
    smart_manager_ = std::make_shared<HidManager>(config_file_);
  }
  else{
    LOGE << "not support this transefer mode";
    return -1;
  }

  if (smart_manager_->Init()) {
    LOGE << "UvcPlugin Init smartManager failed";
    return -1;
  }

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&UvcPlugin::FeedVideoMsg, this, std::placeholders::_1));
  RegisterMsg(TYPE_DROP_IMAGE_MESSAGE, std::bind(&UvcPlugin::FeedVideoDropMsg,
                                                 this, std::placeholders::_1));
  RegisterMsg(TYPE_SMART_MESSAGE,
              std::bind(&UvcPlugin::FeedSmart, this, std::placeholders::_1));
  
  std::lock_guard<std::mutex> lk(video_send_mutex_);
  video_sended_without_recv_count_ = 0;
  encode_thread_.CreatThread(1);
  auto print_timestamp_str = getenv("uvc_print_timestamp");
  if (print_timestamp_str && !strcmp(print_timestamp_str, "ON")) {
    print_timestamp_ = true;
  }
  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
  return 0;
}

int UvcPlugin::Reset() {
  LOGI << __FUNCTION__ << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  return 0;
}

int UvcPlugin::Start() {
  uvc_server_->Start();
  smart_manager_->Start();
  return 0;
}

int UvcPlugin::Stop() {
  LOGI << "UvcPlugin::Stop()";
  stop_flag_ = true;
  uvc_server_->DeInit();
  smart_manager_->Stop();
  return 0;
}

int UvcPlugin::FeedVideoMsg(XProtoMessagePtr msg) {
  encode_thread_.PostTask(
      std::bind(&UvcPlugin::FeedVideo, this, msg));
  return 0;
}

int UvcPlugin::FeedVideoDropMsg(XProtoMessagePtr msg) {
  encode_thread_.PostTask(
      std::bind(&UvcPlugin::FeedVideoDrop, this, msg));
  return 0;
}


int UvcPlugin::FeedSmart(XProtoMessagePtr msg) {
  if (print_timestamp_) {
    auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
    LOGW << "UvcPlugin::FeedSmart time_stamp: " << smart_msg->time_stamp;
  }
  return smart_manager_->FeedSmart(msg, origin_image_width_,
                                   origin_image_height_, dst_image_width_,
                                   dst_image_height_);
}

int UvcPlugin::FeedVideo(XProtoMessagePtr msg) {
  if (UvcServer::IsUvcStreamOn() == 0) {
    return 0;
  }
  UvcServer::SetEncoderRunning(true);
  LOGD << "UvcPlugin Feedvideo";
  VIDEO_FRAME_S pstFrame;
  memset(&pstFrame, 0, sizeof(VIDEO_FRAME_S));
  auto vio_msg = std::dynamic_pointer_cast<VioMessage>(msg);
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStartTime(desc());
  }
  int level = UvcServer::config_->layer_;
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t *>(image.data);
  auto height = src_img->down_scale[level].height;
  auto width = src_img->down_scale[level].width;
  auto y_addr = src_img->down_scale[level].y_vaddr;
  auto uv_addr = src_img->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * src_img->down_scale[level].step;
  auto img_uv_size = img_y_size / 2;

  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.width = src_img->down_scale[0].width;
  pstFrame.stVFrame.height = src_img->down_scale[0].height;
  pstFrame.stVFrame.size =
      src_img->down_scale[0].width * src_img->down_scale[0].height * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = src_img->down_scale[0].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = src_img->down_scale[0].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = src_img->down_scale[0].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = src_img->down_scale[0].c_vaddr;

  origin_image_width_ = src_img->down_scale[0].width;
  origin_image_height_ = src_img->down_scale[0].height;
  dst_image_width_ = src_img->down_scale[level].width;
  dst_image_height_ = src_img->down_scale[level].height;
#endif

#ifdef X3
  auto pym_image = vio_msg->image_[0];
  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.height = pym_image->down_scale[level].height;
  pstFrame.stVFrame.width = pym_image->down_scale[level].width;
  pstFrame.stVFrame.size = pym_image->down_scale[level].height *
                           pym_image->down_scale[level].width * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = (uint32_t)pym_image->down_scale[level].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = (uint32_t)pym_image->down_scale[level].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = (char *)pym_image->down_scale[level].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = (char *)pym_image->down_scale[level].c_vaddr;
  pstFrame.stVFrame.pts = vio_msg->time_stamp_;

  origin_image_width_ = pym_image->down_scale[0].width;
  origin_image_height_ = pym_image->down_scale[0].height;
  dst_image_width_ = pym_image->down_scale[level].width;
  dst_image_height_ = pym_image->down_scale[level].height;
#endif

  if (false == RingQueue<VIDEO_STREAM_S>::Instance().IsValid()) {
    UvcServer::SetEncoderRunning(false);
    return 0;
  }
  if (print_timestamp_) {
    LOGW << "FeedVideo time_stamp:" << vio_msg->time_stamp_;
  }
  if (UvcServer::IsNv12On()) {
    VIDEO_STREAM_S vstream;
    memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
    auto buffer_size = pstFrame.stVFrame.size;
    vstream.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    vstream.pstPack.size = buffer_size;
    memcpy(vstream.pstPack.vir_ptr, pstFrame.stVFrame.vir_ptr[0],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width);
    memcpy(vstream.pstPack.vir_ptr + pstFrame.stVFrame.height * pstFrame.stVFrame.width,
                pstFrame.stVFrame.vir_ptr[1],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width/2);
    RingQueue<VIDEO_STREAM_S>::Instance().Push(vstream);
    return 0;
  }
  auto ts0 = Timer::current_time_stamp();
  int ret = HB_VENC_SendFrame(0, &pstFrame, 0);
  if (ret < 0) {
    LOGE << "HB_VENC_SendFrame 0 error!!!ret " << ret;
    UvcServer::SetEncoderRunning(false);
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    LOGW << "video_sended_without_recv_count: " << video_sended_without_recv_count_;
    return 0;
  } else {
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    ++video_sended_without_recv_count_;
  }

  VIDEO_STREAM_S vstream;
  memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
  ret = HB_VENC_GetStream(0, &vstream, 2000);

  if (UvcServer::config_->h264_encode_time_ == 1) {
    auto ts1 = Timer::current_time_stamp();
    LOGI << "******Encode yuv to h264 cost: " << ts1 - ts0 << "ms";
  }

  if (ret < 0) {
    LOGE << "HB_VENC_GetStream timeout: " << ret;
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    LOGW << "video_sended_without_recv_count: " << video_sended_without_recv_count_;
  } else {
    auto video_buffer = vstream;
    auto buffer_size = video_buffer.pstPack.size;
    HOBOT_CHECK(buffer_size > 5) << "encode bitstream too small";
    int nal_type = -1;
    if ((0 == static_cast<int>(vstream.pstPack.vir_ptr[0])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[1])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[2])) &&
        (1 == static_cast<int>(vstream.pstPack.vir_ptr[3]))) {
      nal_type = static_cast<int>(vstream.pstPack.vir_ptr[4] & 0x1F);
    }
    LOGD << "nal type is " << nal_type;
    video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    if (video_buffer.pstPack.vir_ptr) {
      memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
          buffer_size);
      RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
    }
    HB_VENC_ReleaseStream(0, &vstream);
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    --video_sended_without_recv_count_;
  }
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStopTime(desc());
  }
  UvcServer::SetEncoderRunning(false);
  return 0;
}

int UvcPlugin::FeedVideoDrop(XProtoMessagePtr msg) {
  if (UvcServer::IsUvcStreamOn() == 0) {
    return 0;
  }
  UvcServer::SetEncoderRunning(true);
  VIDEO_FRAME_S pstFrame;
  memset(&pstFrame, 0, sizeof(VIDEO_FRAME_S));
  auto vio_msg = std::dynamic_pointer_cast<VioMessage>(msg);
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStartTime(desc());
  }
  int level = UvcServer::config_->layer_;
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t *>(image.data);
  auto height = src_img->down_scale[level].height;
  auto width = src_img->down_scale[level].width;
  auto y_addr = src_img->down_scale[level].y_vaddr;
  auto uv_addr = src_img->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * src_img->down_scale[level].step;
  auto img_uv_size = img_y_size / 2;

  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.width = src_img->down_scale[0].width;
  pstFrame.stVFrame.height = src_img->down_scale[0].height;
  pstFrame.stVFrame.size =
      src_img->down_scale[0].width * src_img->down_scale[0].height * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = src_img->down_scale[0].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = src_img->down_scale[0].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = src_img->down_scale[0].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = src_img->down_scale[0].c_vaddr;

  origin_image_width_ = src_img->down_scale[0].width;
  origin_image_height_ = src_img->down_scale[0].height;
  dst_image_width_ = src_img->down_scale[level].width;
  dst_image_height_ = src_img->down_scale[level].height;

#endif

#ifdef X3
  auto pym_image = vio_msg->image_[0];
  pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;
  pstFrame.stVFrame.height = pym_image->down_scale[level].height;
  pstFrame.stVFrame.width = pym_image->down_scale[level].width;
  pstFrame.stVFrame.size = pym_image->down_scale[level].height *
                           pym_image->down_scale[level].width * 3 / 2;
  pstFrame.stVFrame.phy_ptr[0] = (uint32_t)pym_image->down_scale[level].y_paddr;
  pstFrame.stVFrame.phy_ptr[1] = (uint32_t)pym_image->down_scale[level].c_paddr;
  pstFrame.stVFrame.vir_ptr[0] = (char *)pym_image->down_scale[level].y_vaddr;
  pstFrame.stVFrame.vir_ptr[1] = (char *)pym_image->down_scale[level].c_vaddr;
  pstFrame.stVFrame.pts = vio_msg->time_stamp_;

  origin_image_width_ = pym_image->down_scale[0].width;
  origin_image_height_ = pym_image->down_scale[0].height;
  dst_image_width_ = pym_image->down_scale[level].width;
  dst_image_height_ = pym_image->down_scale[level].height;
#endif

  if (false == RingQueue<VIDEO_STREAM_S>::Instance().IsValid()) {
    UvcServer::SetEncoderRunning(false);
    return 0;
  }
  if (print_timestamp_) {
    LOGW << "FeedVideoDrop time_stamp:" << vio_msg->time_stamp_;
  }

  if (UvcServer::IsNv12On()) {
    VIDEO_STREAM_S vstream;
    memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
    auto buffer_size = pstFrame.stVFrame.size;
    vstream.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    vstream.pstPack.size = buffer_size;
    memcpy(vstream.pstPack.vir_ptr, pstFrame.stVFrame.vir_ptr[0],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width);
    memcpy(vstream.pstPack.vir_ptr + pstFrame.stVFrame.height * pstFrame.stVFrame.width,
                pstFrame.stVFrame.vir_ptr[1],
                pstFrame.stVFrame.height * pstFrame.stVFrame.width/2);
    RingQueue<VIDEO_STREAM_S>::Instance().Push(vstream);
    return 0;
  }


  int ret = HB_VENC_SendFrame(0, &pstFrame, 0);
  if (ret < 0) {
    LOGE << "HB_VENC_SendFrame 0 error!!!ret " << ret;
    UvcServer::SetEncoderRunning(false);
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    LOGW << "video_sended_without_recv_count: " << video_sended_without_recv_count_;
    return 0;
  } else {
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    ++video_sended_without_recv_count_;
  }

  VIDEO_STREAM_S vstream;
  memset(&vstream, 0, sizeof(VIDEO_STREAM_S));
  ret = HB_VENC_GetStream(0, &vstream, 2000);
  if (ret < 0) {
    LOGE << "HB_VENC_GetStream timeout: " << ret;
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    LOGW << "video_sended_without_recv_count: " << video_sended_without_recv_count_;
  } else {
    auto video_buffer = vstream;
    auto buffer_size = video_buffer.pstPack.size;
    HOBOT_CHECK(buffer_size > 5) << "encode bitstream too small";
    int nal_type = -1;
    if ((0 == static_cast<int>(vstream.pstPack.vir_ptr[0])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[1])) &&
        (0 == static_cast<int>(vstream.pstPack.vir_ptr[2])) &&
        (1 == static_cast<int>(vstream.pstPack.vir_ptr[3]))) {
      nal_type = static_cast<int>(vstream.pstPack.vir_ptr[4] & 0x1F);
    }
    LOGD << "nal type is " << nal_type;
    video_buffer.pstPack.vir_ptr = (char *)calloc(1, buffer_size);
    if (video_buffer.pstPack.vir_ptr) {
      memcpy(video_buffer.pstPack.vir_ptr, vstream.pstPack.vir_ptr,
          buffer_size);
      RingQueue<VIDEO_STREAM_S>::Instance().Push(video_buffer);
    }
    HB_VENC_ReleaseStream(0, &vstream);
    std::lock_guard<std::mutex> lk(video_send_mutex_);
    --video_sended_without_recv_count_;
  }
  if (vio_msg != nullptr && vio_msg->profile_ != nullptr) {
    vio_msg->profile_->UpdatePluginStopTime(desc());
  }
  UvcServer::SetEncoderRunning(false);
  return 0;
}
}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
