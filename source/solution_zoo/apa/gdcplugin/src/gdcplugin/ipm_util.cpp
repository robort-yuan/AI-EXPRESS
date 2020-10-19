/* @Description: impolement of ipm util
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-05 18:00:55
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-08 13:52:53
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include "gdcplugin/ipm_util.h"
#include "horizon/vision_type/vision_type.h"
#include "horizon/vision_type/vision_type.hpp"
#include "horizon/vision_type/vision_type_util.h"
#include "hobotlog/hobotlog.hpp"
#include "opencv2/opencv.hpp"
#include "hobotxsdk/xstream_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace gdcplugin {

int IpmUtil::Init(const std::string cfg0, const std::string cfg1,
                  const std::string cfg2, const std::string cfg3,
                  std::vector<int> c2d, uint32_t source_num) {
  channel2direction_ = c2d;
  source_num_ = source_num;
  auto ret =
    InitIPM(cfg0.c_str(), cfg1.c_str(), cfg2.c_str(), cfg3.c_str(), 256, 512);
  if (ret != 0) {
    LOGE << "InitIPM failed: " << ret;
    return -1;
  }
  return 0;
}

int IpmUtil::GenIPMImage(std::vector<std::shared_ptr<PymImageFrame>> pym_imgs,
                         std::vector<std::shared_ptr<CVImageFrame>> &outs) {
  std::vector<HOBOT_IMAGE> inputs, outputs;
  inputs.resize(source_num_);
  outputs.resize(source_num_);
  for (size_t i = 0; i < pym_imgs.size(); ++i) {
    auto chn = pym_imgs[i]->channel_id;
    inputs[i].image_direction_ = IMAGE_DIRECTION(channel2direction_[chn]);
    outputs[i].image_direction_ = IMAGE_DIRECTION(channel2direction_[chn]);
    outputs[i].data_ =
      reinterpret_cast<unsigned char *>(malloc((1280 * 720 * 3 >> 1) *
                                        sizeof(char)));

    // put pym img input inputs[i]
    auto cur_img = pym_imgs[i];
    auto height = cur_img->Height();
    auto width = cur_img->Width();
    HOBOT_CHECK(height == 720 && width == 1280)
      << "only support 1280 * 720 image, input width: " << width
      << ", height: " << height;
    auto *img_addr =
      reinterpret_cast<unsigned char *>(malloc((height * width * 3 >> 1) *
                                        sizeof(char)));
    memcpy(img_addr, reinterpret_cast<uint8_t *>(cur_img->Data()),
           cur_img->DataSize());
    memcpy(img_addr + cur_img->DataSize(),
           reinterpret_cast<uint8_t *>(cur_img->DataUV()),
                                       cur_img->DataUVSize());
    inputs[i].data_ = img_addr;

    auto ret = GetIPMImage(&inputs[i], &outputs[i], 1280, 720, 256, 512);
    // free input data
    {
      free(inputs[i].data_);
      inputs[i].data_ = nullptr;
    }
    if (ret != 0) {
      LOGE << "Failed to get IPM image, code: " << ret;
      return ret;
    }

    // binary ipm data to CVImageFrame
    cv::Mat ipm;
    ipm.create(512 * 3 / 2, 256, CV_8UC1);
    memcpy(ipm.data, outputs[i].data_, 256 * 512 * 3 / 2 * sizeof(char));
    // free output data
    {
      free(outputs[i].data_);
      outputs[i].data_ = nullptr;
    }
    outs[i]->img = ipm;
    outs[i]->pixel_format =
      HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
  }
  return 0;
}

}  // namespace gdcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon


