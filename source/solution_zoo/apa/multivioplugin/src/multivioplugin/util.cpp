/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "multivioplugin/util.h"
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "opencv2/opencv.hpp"
#include "hobotlog/hobotlog.hpp"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_msg.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type_util.h"
#include "horizon/vision_type/vision_type.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {


#if defined(J3_MEDIA_LIB)
// todo, need to update by hangjun.yang
void Convert(pym_buffer_t *pym_buffer, PymImageFrame &pym_img) {
  if (nullptr == pym_buffer) {
    return;
  }
  pym_img.ds_pym_total_layer = DOWN_SCALE_MAX;
  pym_img.us_pym_total_layer = UP_SCALE_MAX;
  pym_img.frame_id = pym_buffer->pym_img_info.frame_id;
  pym_img.time_stamp = pym_buffer->pym_img_info.time_stamp;
  pym_img.context = static_cast<void *>(pym_buffer);
  for (int i = 0; i < DOWN_SCALE_MAX; ++i) {
    address_info_t *pym_addr = NULL;
    if (i % 4 == 0) {
      pym_addr = reinterpret_cast<address_info_t *>(&pym_buffer->pym[i / 4]);
    } else {
      pym_addr = reinterpret_cast<address_info_t *>(
          &pym_buffer->pym_roi[i / 4][i % 4 - 1]);
    }
    LOGD << "dxd1 : " << pym_addr->width;
    pym_img.down_scale[i].width = pym_addr->width;
    pym_img.down_scale[i].height = pym_addr->height;
    pym_img.down_scale[i].stride = pym_addr->stride_size;
    pym_img.down_scale[i].y_paddr = pym_addr->paddr[0];
    pym_img.down_scale[i].c_paddr = pym_addr->paddr[1];
    pym_img.down_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[0]);
    pym_img.down_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_addr->addr[1]);
  }
  for (int i = 0; i < UP_SCALE_MAX; ++i) {
    pym_img.up_scale[i].width = pym_buffer->us[i].width;
    pym_img.up_scale[i].height = pym_buffer->us[i].height;
    pym_img.up_scale[i].stride = pym_buffer->us[i].stride_size;
    pym_img.up_scale[i].y_paddr = pym_buffer->us[i].paddr[0];
    pym_img.up_scale[i].c_paddr = pym_buffer->us[i].paddr[1];
    pym_img.up_scale[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[0]);
    pym_img.up_scale[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->us[i].addr[1]);
  }
  for (int i = 0; i < DOWN_SCALE_MAIN_MAX; ++i) {
    LOGD << "dxd2 : " << pym_buffer->pym[i].width;
    pym_img.down_scale_main[i].width = pym_buffer->pym[i].width;
    pym_img.down_scale_main[i].height = pym_buffer->pym[i].height;
    pym_img.down_scale_main[i].stride = pym_buffer->pym[i].stride_size;
    pym_img.down_scale_main[i].y_paddr = pym_buffer->pym[i].paddr[0];
    pym_img.down_scale_main[i].c_paddr = pym_buffer->pym[i].paddr[1];
    pym_img.down_scale_main[i].y_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[0]);
    pym_img.down_scale_main[i].c_vaddr =
        reinterpret_cast<uint64_t>(pym_buffer->pym[i].addr[1]);
  }
}
#endif  // J3_MEDIA_LIB

int HorizonConvertImage(HorizonVisionImage *in_img, HorizonVisionImage *dst_img,
                        HorizonVisionPixelFormat dst_format) {
  if (!in_img || !dst_img)
    return kHorizonVisionErrorParam;
  dst_img->width = in_img->width;
  dst_img->height = in_img->height;
  dst_img->stride = in_img->stride;
  dst_img->stride_uv = in_img->stride_uv;
  dst_img->pixel_format = dst_format;
  if (dst_format != in_img->pixel_format) {
    switch (in_img->pixel_format) {
    case kHorizonVisionPixelFormatRawRGB: {
      cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB565: {
        auto dst_data_size = in_img->data_size / 3 * 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, dst_img->data);
        cv::cvtColor(rgb888, rgb565, CV_BGR2BGR565);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        dst_img->data_size = in_img->data_size;
        dst_img->data =
            reinterpret_cast<uint8_t *>(std::malloc(dst_img->data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb888, bgr888, CV_RGB2BGR);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawRGB565: {
      cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = (in_img->data_size >> 1) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb565, rgb888, CV_BGR5652BGR);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->data_size >> 1) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(rgb565, bgr888, CV_BGR5652RGB);
      } break;

      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawBGR: {
      cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = in_img->data_size;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgr888, rgb888, CV_BGR2RGB);
      } break;
      case kHorizonVisionPixelFormatRawRGB565: {
        auto dst_data_size = in_img->data_size / 3 * 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb565(in_img->height, in_img->width, CV_8UC2, dst_img->data);
        cv::cvtColor(bgr888, rgb565, CV_RGB2BGR565);
      } break;
      case kHorizonVisionPixelFormatRawNV12: {
        auto dst_data_size = (in_img->height * in_img->width) * 3 / 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat yuv420_mat;
        cv::cvtColor(bgr888, yuv420_mat, cv::COLOR_BGR2YUV_I420);
        // copy y data
        int y_size = in_img->height * in_img->width;
        auto *yuv420_ptr = yuv420_mat.ptr<uint8_t>();
        memcpy(dst_img->data, yuv420_ptr, y_size);
        // copy uv data
        int uv_stride = in_img->width * in_img->height / 4;
        uint8_t *uv_data = dst_img->data + y_size;
        for (int i = 0; i < uv_stride; ++i) {
          *(uv_data++) = *(yuv420_ptr + y_size + i);
          *(uv_data++) = *(yuv420_ptr + y_size + uv_stride + i);
        }
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawBGRA: {
      cv::Mat bgra(in_img->height, in_img->width, CV_8UC4, in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawRGB: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgb888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgra, rgb888, CV_BGRA2RGB);
      } break;
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr888(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(bgra, bgr888, CV_BGRA2BGR);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    case kHorizonVisionPixelFormatRawNV12: {
      cv::Mat nv12((in_img->height * 3) >> 1, in_img->width, CV_8UC1,
                   in_img->data);
      switch (dst_format) {
      case kHorizonVisionPixelFormatRawBGR: {
        auto dst_data_size = (in_img->height * in_img->width) * 3;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgr(in_img->height, in_img->width, CV_8UC3, dst_img->data);
        cv::cvtColor(nv12, bgr, CV_YUV2BGR_NV12);
      } break;
      case kHorizonVisionPixelFormatRawBGRA: {
        auto dst_data_size = (in_img->height * in_img->width) << 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat bgra(in_img->height, in_img->width, CV_8UC4, dst_img->data);
        cv::cvtColor(nv12, bgra, CV_YUV2BGRA_NV12);
      } break;
      case kHorizonVisionPixelFormatRawRGBA: {
        auto dst_data_size = (in_img->height * in_img->width) << 2;
        dst_img->data_size = dst_data_size;
        dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(dst_data_size));
        cv::Mat rgba(in_img->height, in_img->width, CV_8UC4, dst_img->data);
        cv::cvtColor(nv12, rgba, CV_YUV2RGBA_NV12);
      } break;
      default:
        return kHorizonVisionErrorNoImpl;
      }
    } break;
    default:
      return kHorizonVisionErrorNoImpl;
    }
  } else {
    dst_img->data_size = in_img->data_size;
    dst_img->data = reinterpret_cast<uint8_t *>(std::malloc(in_img->data_size));
    std::memcpy(dst_img->data, in_img->data, in_img->data_size);
  }
  return kHorizonVisionSuccess;
}

int HorizonSave2File(HorizonVisionImage *img, const char *file_path) {
  if (!img || !file_path)
    return kHorizonVisionErrorParam;
  if (0 == strlen(file_path))
    return kHorizonVisionErrorParam;
  if (kHorizonVisionPixelFormatRawBGR == img->pixel_format) {
    cv::Mat bgr_img_mat(img->height, img->width, CV_8UC3, img->data);
    cv::imwrite(file_path, bgr_img_mat);
    return kHorizonVisionSuccess;
  }
  HorizonVisionImage *bgr_img;
  HorizonVisionAllocImage(&bgr_img);
  int ret = HorizonConvertImage(img, bgr_img, kHorizonVisionPixelFormatRawBGR);
  if (ret != kHorizonVisionSuccess)
    return ret;
  // save to file
  cv::Mat bgr_img_mat(bgr_img->height, bgr_img->width, CV_8UC3, bgr_img->data);
  cv::imwrite(file_path, bgr_img_mat);
  HorizonVisionFreeImage(bgr_img);
  return kHorizonVisionSuccess;
}

int HorizonFillFromFile(const char *file_path, HorizonVisionImage **ppimg) {
  if (!ppimg) {
    return kHorizonVisionErrorParam;
  }
  if (*ppimg || !file_path)
    return kHorizonVisionErrorParam;

  if (!strcmp(file_path, "")) {
    return kHorizonVisionErrorParam;
  }
  if (access(file_path, F_OK) != 0) {
    LOGE << "file not exist: " << file_path;
    return kHorizonVisionOpenFileFail;
  }

  try {
    auto bgr_mat = cv::imread(file_path);
    if (!bgr_mat.data) {
      LOGF << "Failed to call imread for " << file_path;
      return kHorizonVisionFailure;
    }
    HorizonVisionAllocImage(ppimg);
    auto &img = *ppimg;
    img->pixel_format = kHorizonVisionPixelFormatRawBGR;
    img->data_size = static_cast<uint32_t>(bgr_mat.total() * 3);
    // HorizonVisionFreeImage call std::free to free data
    img->data = reinterpret_cast<uint8_t *>(
        std::calloc(img->data_size, sizeof(uint8_t)));
    std::memcpy(img->data, bgr_mat.data, img->data_size);
    img->width = static_cast<uint32_t>(bgr_mat.cols);
    img->height = static_cast<uint32_t>(bgr_mat.rows);
    img->stride = static_cast<uint32_t>(bgr_mat.cols);
    img->stride_uv = static_cast<uint32_t>(bgr_mat.cols);
  } catch (const cv::Exception &e) {
    LOGF << "Exception to call imread for " << file_path;
    return kHorizonVisionOpenFileFail;
  }
  return kHorizonVisionSuccess;
}

// 补全图像，保证图像按照规定分辨率输入
int PadImage(HorizonVisionImage *img, uint32_t dst_width,
                         uint32_t dst_height) {
  if (!img) {
    return kHorizonVisionErrorParam;
  }
  if (img->height == dst_height && img->width == dst_width) {
    return kHorizonVisionSuccess;
  }
  cv::Mat in_img(img->height, img->width, CV_8UC3);
  memcpy(in_img.data, img->data, img->data_size);
  HOBOT_CHECK(!in_img.empty());
  uint32_t dst_data_size = dst_width * dst_height * 3;
  cv::Mat out_img = cv::Mat(dst_height, dst_width, CV_8UC3, cv::Scalar::all(0));
  if (img->width > dst_width || img->height > dst_height) {
    auto src_width = static_cast<float>(img->width);
    auto src_height = static_cast<float>(img->height);
    auto aspect_ratio = src_width / src_height;
    auto dst_ratio = static_cast<float>(dst_width) / dst_height;
    uint32_t resized_width = -1;
    uint32_t resized_height = -1;
    // 等比缩放
    if (aspect_ratio >= dst_ratio) {
      resized_width = dst_width;
      resized_height =
          static_cast<uint32_t>(src_height * dst_width / src_width);
    } else {
      resized_width =
          static_cast<uint32_t>(src_width * dst_height / src_height);
      resized_height = dst_height;
    }
    cv::resize(in_img, in_img, cv::Size(resized_width, resized_height));
  }

  // 复制到目标图像中间
  in_img.copyTo(out_img(cv::Rect((dst_width - in_img.cols) / 2,
                                 (dst_height - in_img.rows) / 2, in_img.cols,
                                 in_img.rows)));
  HorizonVisionCleanImage(img);
  img->data =
      reinterpret_cast<uint8_t *>(std::calloc(dst_data_size, sizeof(uint8_t)));
  memcpy(img->data, out_img.data, dst_data_size);
  img->data_size = dst_data_size;
  img->width = dst_width;
  img->height = dst_height;
  img->stride = dst_width;
  img->stride_uv = dst_width;
  return kHorizonVisionSuccess;
}

  // 将指定路径的图像转换为HorizonVisionImageFrame格式
HorizonVisionImageFrame *GetImageFrame(const std::string &path) {
  HorizonVisionImage *bgr_img = nullptr;
  std::string image_path = path;
  // avoid windows system line break
  if (!image_path.empty() && image_path.back() == '\r') {
    image_path.erase(image_path.length() - 1);
  }
  auto res = HorizonFillFromFile(path.c_str(), &bgr_img);
  if (res != 0) {
    LOGE << "Failed to load image " << path << ", error code is " << res;
    return nullptr;
  }
  HOBOT_CHECK(bgr_img);
  static uint64_t frame_id = 0;
  HorizonVisionImageFrame *frame = nullptr;
  HorizonVisionAllocImageFrame(&frame);
  frame->channel_id = 0;
  frame->frame_id = frame_id++;
  frame->time_stamp = static_cast<uint64_t>(std::time(nullptr));
  // 转换图像数据
  HorizonConvertImage(bgr_img, &frame->image, kHorizonVisionPixelFormatRawBGR);
  HorizonVisionFreeImage(bgr_img);
  return frame;
}

  std::shared_ptr<ImageVioMessage> Image2ImageMessageInput(
      const HorizonVisionImage *image);

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
