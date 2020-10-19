/*
 * @Description: implement of ipm test
 * @Author: zhe.sun@horizon.ai
 * @Date: 2020-09-08 21:05:07
 * @LastEditors: zhe.sun@horizon.ai
 * @LastEditTime: 2020-09-09 13:50:18
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "PredictMethod/PredictMethod.h"
#include "PostProcessMethod/PostProcessMethod.h"
#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "opencv2/opencv.hpp"
#include "horizon/vision_type/vision_type.hpp"

using xstream::BaseData;
using xstream::BaseDataPtr;
using xstream::BaseDataVector;
using xstream::InputData;
using xstream::InputDataPtr;
using xstream::XStreamData;
using xstream::InputParamPtr;

using xstream::PredictMethod;
using xstream::PostProcessMethod;
using hobot::vision::ImageFrame;
using hobot::vision::CVImageFrame;


int main(int argc, char **argv) {
  SetLogLevel(HOBOT_LOG_DEBUG);
  if (argc < 4) {
    LOGE << "exec imp_path predict_config_file post_config_file" << std::endl;
  }
  std::string ipm_path = argv[1];
  std::string predict_config_file = argv[2];
  std::string post_config_file = argv[3];

  PredictMethod predict_method;
  predict_method.Init(predict_config_file);
  PostProcessMethod post_method;
  post_method.Init(post_config_file);

  // prepare img
  cv::Mat yuvImg;
  {
    std::ifstream ifs(ipm_path.c_str(), std::ios::in | std::ios::binary);
    if (!ifs) {
      LOGE << "open img failed";
      return 0;
    }
    ifs.seekg(0, std::ios::end);
    int img_length = ifs.tellg();
    int height = 512, width = 256;
    HOBOT_CHECK(img_length == height * 3 / 2 * width);
    ifs.seekg(0, std::ios::beg);
    char *img_data = new char[sizeof(char) * img_length];
    ifs.read(img_data, img_length);
    ifs.close();

    yuvImg.create(height * 3 / 2, width, CV_8UC1);
    memcpy(yuvImg.data, img_data, img_length * sizeof(unsigned char));
  }
  auto cv_image_frame_ptr = std::make_shared<CVImageFrame>();
  cv_image_frame_ptr->img = yuvImg;
  cv_image_frame_ptr->pixel_format =
      HorizonVisionPixelFormat::kHorizonVisionPixelFormatRawNV12;
  auto xstream_img =
      std::make_shared<XStreamData<std::shared_ptr<ImageFrame>>>();
  xstream_img->value = cv_image_frame_ptr;

  std::vector<std::vector<BaseDataPtr>> input;
  std::vector<xstream::InputParamPtr> param;
  input.resize(1);
  input[0].push_back(xstream_img);
  std::vector<std::vector<BaseDataPtr>> predict_output =
      predict_method.DoProcess(input, param);

  post_method.DoProcess(predict_output, param);

  predict_method.Finalize();
  post_method.Finalize();
  return 0;
}
