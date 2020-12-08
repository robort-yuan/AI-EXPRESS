/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-02 04:05:52
 * @Version: v0.0.1
 * @Brief: implemenation of converter.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-08-02 06:26:01
 */
#include <string>
#include "websocketplugin/convert.h"
#include "websocketplugin/websocketconfig.h"
#include "smartplugin/smartplugin.h"
#include "hobotxsdk/xstream_data.h"
#include "horizon/vision/util.h"
#include "hobotlog/hobotlog.hpp"
#include "turbojpeg.h"
// #include "hb_vio_interface.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace websocketplugin {

using horizon::vision::util::ImageFrameConversion;
using horizon::vision::xproto::smartplugin::CustomSmartMessage;
using horizon::vision::xproto::smartplugin::VehicleSmartMessage;

int Convertor::GetYUV(cv::Mat &yuv_img, VioMessage *vio_msg, int level) {
  LOGV << __FUNCTION__;
  if (!vio_msg || vio_msg->num_ == 0)
    return -1;
#ifdef X2
  auto image = vio_msg->image_[0]->image;
  img_info_t *src_img = reinterpret_cast<img_info_t * >(image.data);
  auto height = src_img->down_scale[level].height;
  auto width = src_img->down_scale[level].width;
  auto y_addr = src_img->down_scale[level].y_vaddr;
  auto uv_addr = src_img->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * src_img->down_scale[level].step;
  auto img_uv_size = img_y_size / 2;
#endif

#ifdef X3
  auto pym_image = vio_msg->image_[0];
  auto height = pym_image->down_scale[level].height;
  auto width = pym_image->down_scale[level].width;
  auto y_addr = pym_image->down_scale[level].y_vaddr;
  auto uv_addr = pym_image->down_scale[level].c_vaddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * pym_image->down_scale[level].stride;
  auto img_uv_size = img_y_size / 2;
#endif

  yuv_img.create(height * 3 / 2, width, CV_8UC1);
  memcpy(yuv_img.data, reinterpret_cast<uint8_t*>(y_addr), img_y_size);
  memcpy(yuv_img.data + height * width, reinterpret_cast<uint8_t*>
      (uv_addr), img_uv_size);
  HOBOT_CHECK(!yuv_img.empty())
      << "width = " << width << ", height = " << height;

#if 0
  static bool first = true;
  if (first) {
    std::fstream fout("1.yuv", std::ios::out | std::ios::binary);
    fout.write((const char *)yuv_img.data, width * height * 1.5);
    fout.close();
    first = false;
  }
#endif
  return 0;
}

#ifdef X3_MEDIA_CODEC
int Convertor::GetYUV(iot_venc_src_buf_t *frame_buf, VioMessage *vio_msg,
        int level, int use_vb) {
  LOGI << "websocketplugin x3 mediacodec: " << __FUNCTION__;
  if (!vio_msg || vio_msg->num_ == 0)
    return -1;
  auto pym_image = vio_msg->image_[0];
  auto height = pym_image->down_scale[level].height;
  auto width = pym_image->down_scale[level].width;
  auto stride = pym_image->down_scale[level].stride;
  auto y_vaddr = pym_image->down_scale[level].y_vaddr;
  auto y_paddr = pym_image->down_scale[level].y_paddr;
  auto c_vaddr = pym_image->down_scale[level].c_vaddr;
  auto c_paddr = pym_image->down_scale[level].c_paddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;
  auto img_y_size = height * stride;
  auto img_uv_size = img_y_size / 2;

  if (use_vb) {
    HOBOT_CHECK(frame_buf != nullptr);
    HOBOT_CHECK(frame_buf->frame_info.vir_ptr[0] != NULL);
    HOBOT_CHECK(frame_buf->frame_info.vir_ptr[1] != NULL);
    memcpy(frame_buf->frame_info.vir_ptr[0],
        reinterpret_cast<uint8_t*>(y_vaddr), img_y_size);
    memcpy(frame_buf->frame_info.vir_ptr[1],
        reinterpret_cast<uint8_t*>(c_vaddr), img_uv_size);
  } else {
    frame_buf->frame_info.width = width;
    frame_buf->frame_info.height = height;
    frame_buf->frame_info.stride = stride;
    frame_buf->frame_info.size = stride * height * 3 / 2;
    frame_buf->frame_info.vir_ptr[0] = reinterpret_cast<char *>(y_vaddr);
    frame_buf->frame_info.phy_ptr[0] = (uint32_t)y_paddr;
    frame_buf->frame_info.vir_ptr[1] = reinterpret_cast<char *>(c_vaddr);
    frame_buf->frame_info.phy_ptr[1] = (uint32_t)c_paddr;
    frame_buf->frame_info.pix_format = HB_PIXEL_FORMAT_NV12;
  }

#if 0  // dump yuv data
  static bool first = true;
  if (first) {
      static int frame_id = 0;
      std::string file_name = "out_stream_" +
          std::to_string(frame_id++) + ".yuv";
      std::fstream fout(file_name, std::ios::out | std::ios::binary);
      fout.write((const char *)img_y_addr, img_y_size);
      fout.write((const char *)img_uv_addr, img_uv_size);
      fout.close();
      first = false;  // only dump a yuv
  }
#endif
#if 0  // debug test
  for (std::size_t i = 0; i < vio_msg->image_.size(); i++) {
    pym_image = vio_msg->image_[i];
    auto pipe_id = pym_image->channel_id;
    auto pym_buffer = static_cast<pym_buffer_t*>(pym_image->context);
    auto frame_id = pym_buffer->pym_img_info.frame_id;
    auto ts = pym_buffer->pym_img_info.time_stamp;
    LOGW << "pipe_id: " << pipe_id << " frame_id: "
      << frame_id << " ts: " << ts;
  }
#endif


  return 0;
}
#endif

bool Convertor::YUV2JPG(std::vector<uchar> &img_buf,
    cv::Mat &yuv_img, int quality) {
  std::vector<int> params;
  params.push_back(cv::IMWRITE_JPEG_QUALITY);
  params.push_back(quality);

  cv::Mat img;
  cv::cvtColor(yuv_img, img, cv::COLOR_YUV2BGR_NV12);
  return cv::imencode(".jpg", img, img_buf, params);
}

int Convertor::PackSmartMsg(std::string &data, SmartMessage *smart_msg,
                            int type, int ori_w, int ori_h, int dst_w,
                            int dst_h) {
  if (!smart_msg)
    return -1;
  switch ((WebsocketConfig::SmartType)type) {
  case WebsocketConfig::SMART_FACE:
  case WebsocketConfig::SMART_BODY: {
      auto msg = dynamic_cast<CustomSmartMessage*>(smart_msg);
      if (msg)
        data = msg->Serialize(ori_w, ori_h, dst_w, dst_h);
    break;
  }
  case WebsocketConfig::SMART_VEHICLE: {
    auto msg = dynamic_cast<VehicleSmartMessage*>(smart_msg);
    if (msg) {
      data = msg->Serialize(ori_w, ori_h, dst_w, dst_h);
    }
    break;
  }
  default:
    return -1;
  }
  return 0;
}
}  // namespace websocketplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
