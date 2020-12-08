/**
 * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @file votmodule.cpp
 * @brief vot module
 * @author kairui.wang
 * @email kairui.wang@horizon.ai
 * @date 2020/07/22
 */
#include "votmodule.h"

#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "hb_vot.h"
#include "hobotlog/hobotlog.hpp"
#include "libyuv/convert.h"
#include "opencv2/opencv.hpp"
#include "smartplugin_box/displayinfo.h"

// #define PRINTF_USE_TIME

namespace horizon {
namespace vision {
VotModule::VotModule()
    : group_id_(-1), timeout_(40), display_mode_(0), channel_num_(0) {}

VotModule::~VotModule() { ; }

int VotModule::Init(uint32_t group_id, const PipeModuleInfo *module_info,
                    const smart_vo_cfg_t &smart_vo_cfg) {
  vo_plot_cfg_ = smart_vo_cfg;
  LOGW << "vo_plot_cfg  box_face_thr: " << vo_plot_cfg_.box_face_thr
       << "  box_head_thr:" << vo_plot_cfg_.box_head_thr
       << "  box_body_thr:" << vo_plot_cfg_.box_body_thr
       << "  lmk_thr:" << vo_plot_cfg_.lmk_thr
       << "  kps_thr:" << vo_plot_cfg_.kps_thr
       << "  box_veh_thr:" << vo_plot_cfg_.box_veh_thr
       << "  plot_fps:" << vo_plot_cfg_.plot_fps;

  int ret = 0;
  image_height_ = 540;
  image_width_ = 960;

  VOT_VIDEO_LAYER_ATTR_S stLayerAttr;
  VOT_CHN_ATTR_S stChnAttr;
  VOT_CROP_INFO_S cropAttrs;
  VOT_PUB_ATTR_S devAttr;

  devAttr.enIntfSync = VOT_OUTPUT_1920x1080;
  devAttr.u32BgColor = 0x108080;
  devAttr.enOutputMode = HB_VOT_OUTPUT_BT1120;
  ret = HB_VOT_SetPubAttr(0, &devAttr);
  if (ret) {
    printf("HB_VOT_SetPubAttr failed\n");
    return -1;
  }
  ret = HB_VOT_Enable(0);
  if (ret) printf("HB_VOT_Enable failed.\n");

  ret = HB_VOT_GetVideoLayerAttr(0, &stLayerAttr);
  if (ret) {
    printf("HB_VOT_GetVideoLayerAttr failed.\n");
  }

  stLayerAttr.stImageSize.u32Width = 1920;
  stLayerAttr.stImageSize.u32Height = 1080;
  stLayerAttr.panel_type = 0;
  stLayerAttr.rotate = 0;
  stLayerAttr.dithering_flag = 0;
  stLayerAttr.dithering_en = 0;
  stLayerAttr.gamma_en = 0;
  stLayerAttr.hue_en = 0;
  stLayerAttr.sat_en = 0;
  stLayerAttr.con_en = 0;
  stLayerAttr.bright_en = 0;
  stLayerAttr.theta_sign = 0;
  stLayerAttr.contrast = 0;
  stLayerAttr.theta_abs = 0;
  stLayerAttr.saturation = 0;
  stLayerAttr.off_contrast = 0;
  stLayerAttr.off_bright = 0;
  stLayerAttr.user_control_disp = 0;
  stLayerAttr.big_endian = 0;
  stLayerAttr.display_addr_type = 2;
  stLayerAttr.display_addr_type_layer1 = 2;
  ret = HB_VOT_SetVideoLayerAttr(0, &stLayerAttr);
  if (ret) printf("HB_VOT_SetVideoLayerAttr failed.\n");

  ret = HB_VOT_EnableVideoLayer(0);
  if (ret) printf("HB_VOT_EnableVideoLayer failed.\n");

  stChnAttr.u32Priority = 2;
  stChnAttr.s32X = 0;
  stChnAttr.s32Y = 0;
  stChnAttr.u32SrcWidth = 1920;
  stChnAttr.u32SrcHeight = 1080;
  stChnAttr.u32DstWidth = 1920;
  stChnAttr.u32DstHeight = 1080;
  buffer_ = static_cast<char *>(malloc(1920 * 1080 * 3 / 2));

  ret = HB_VOT_SetChnAttr(0, 0, &stChnAttr);
  printf("HB_VOT_SetChnAttr 0: %d\n", ret);
  cropAttrs.u32Width = stChnAttr.u32DstWidth;
  cropAttrs.u32Height = stChnAttr.u32DstHeight;
  ret = HB_VOT_SetChnCrop(0, 0, &cropAttrs);

  ParseLogoImg("./video_box/configs/jisuanhe-top@2x.png",
               "./video_box/configs/aionhorizon@2x.png");
  ParseBottomLogoImg("./video_box/configs/aionhorizon-left@1x.png",
                     "./video_box/configs/aionhorizon-right@1x.png");
  return ret;
}

int VotModule::PlotSmartData(cv::Mat &bgr, bool face, bool head, bool body,
                             bool kps, bool veh, VotData *vot_data,
                             const xstream::OutputDataPtr &xstream_out) {
  uint32_t width_tmp = vot_data->width;
  uint32_t height_tmp = vot_data->height;
  char *buffer = static_cast<char *>(malloc(width_tmp * height_tmp * 3 / 2));
  for (uint32_t i = 0; i < height_tmp; ++i) {
    memcpy(buffer + i * width_tmp, vot_data->y_virtual_addr + i * width_tmp,
           width_tmp);
  }

  for (uint32_t i = 0; i < (height_tmp / 2); ++i) {
    memcpy(buffer + (i + height_tmp) * width_tmp,
           vot_data->uv_virtual_addr + i * width_tmp, width_tmp);
  }

  uint8_t *img_addr = reinterpret_cast<uint8_t *>(buffer);
  cv::cvtColor(cv::Mat(height_tmp * 3 / 2, width_tmp, CV_8UC1, img_addr), bgr,
               CV_YUV2BGR_NV12);
  free(buffer);

  const static std::map<std::string, decltype(CV_RGB(255, 0, 0))> d_color =  // NOLINT
          {{"id", CV_RGB(255, 0, 0)},
           {"face", CV_RGB(255, 128, 0)},
           {"head", CV_RGB(255, 128, 0)},
           {"body", CV_RGB(255, 128, 0)},
           {"lmk", CV_RGB(0, 245, 255)},
           {"kps", CV_RGB(0, 245, 255)},
           {"vehicle", CV_RGB(255, 128, 0)},
           {"plate", CV_RGB(0, 255, 0)},
           {"fps", CV_RGB(0, 255, 0)}
          };

  const static std::map<std::string, int> d_thickness =  // NOLINT
          {{"id", 2},
           {"face", 2},
           {"head", 2},
           {"body", 2},
           {"lmk", 2},
           {"kps", 2},
           {"kps_line", 2},
           {"vehicle", 2},
           {"plate", 1},
           {"fps", 2}
          };

  int x_offset = 0;
  int y_offset = 0;
  float x_scale = 0.5;
  float y_scale = 0.5;
  x_scale = static_cast<float>(vot_data->width / 1920.0);
  y_scale = static_cast<float>(vot_data->height / 1080.0);
  auto plot_box =
      [](cv::Mat &bgr_,
         const std::shared_ptr<xstream::XStreamData<hobot::vision::BBox>> &box,
         float score, decltype(CV_RGB(255, 0, 0)) color, double x_scale0,
         double y_scale0, int x0, int y0, int thickness = 1) {
        if (box->value.score < score) return;
        cv::rectangle(bgr_,
                      cv::Point(box->value.x1 * x_scale0 + x0,
                                box->value.y1 * y_scale0 + y0),
                      cv::Point(box->value.x2 * x_scale0 + x0,
                                box->value.y2 * y_scale0 + y0),
                      color, thickness);
      };
  for (const auto &output : xstream_out->datas_) {
    LOGW << output->name_ << ", type is " << output->type_;

    if (face) {
      if (output->name_ == "face_final_box") {
        auto boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
        LOGW << "box size: " << boxes->datas_.size();
        for (size_t i = 0; i < boxes->datas_.size(); ++i) {
          auto box = std::static_pointer_cast<
              xstream::XStreamData<hobot::vision::BBox>>(boxes->datas_[i]);
          plot_box(bgr, box, vo_plot_cfg_.box_face_thr, d_color.at("face"),
                   x_scale, y_scale, x_offset, y_offset,
                   d_thickness.at("face"));
        }
      }
      if (output->name_ == "lmk") {
        using XRocLandmarks = xstream::XStreamData<hobot::vision::Landmarks>;
        xstream::BaseDataVector *lmk =
            dynamic_cast<xstream::BaseDataVector *>(output.get());
        for (auto base : lmk->datas_) {
          auto xroc_lmk = dynamic_cast<XRocLandmarks *>(base.get());
          auto lmks = xroc_lmk->value;
          for (const auto &val : lmks.values) {
            if (val.score > vo_plot_cfg_.lmk_thr) {
              cv::circle(bgr,
                         cv::Point(val.x * x_scale + x_offset,
                                   val.y * y_scale + y_offset),
                         1, d_color.at("lmk"), d_thickness.at("lmk"));
            }
          }
        }
      }
    }

    if (head && output->name_ == "head_final_box") {
      auto boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGW << "box size: " << boxes->datas_.size();
      for (size_t i = 0; i < boxes->datas_.size(); ++i) {
        auto box =
            std::static_pointer_cast<xstream::XStreamData<hobot::vision::BBox>>(
                boxes->datas_[i]);
        plot_box(bgr, box, vo_plot_cfg_.box_head_thr, d_color.at("head"),
                 x_scale, y_scale, x_offset, y_offset, d_thickness.at("head"));
      }
    }

    if (body) {
      if (output->name_ == "body_final_box") {
        auto boxes = dynamic_cast<xstream::BaseDataVector *>(output.get());
        LOGW << "box size: " << boxes->datas_.size();
        for (size_t i = 0; i < boxes->datas_.size(); ++i) {
          auto box = std::static_pointer_cast<
              xstream::XStreamData<hobot::vision::BBox>>(boxes->datas_[i]);
          plot_box(bgr, box, vo_plot_cfg_.box_body_thr, d_color.at("body"),
                   x_scale, y_scale, x_offset, y_offset,
                   d_thickness.at("body"));
        }
      }
    }

    if (kps) {
      if (output->name_ == "kps") {
        auto lmks = dynamic_cast<xstream::BaseDataVector *>(output.get());
        LOGW << "kps size: " << lmks->datas_.size();
        for (size_t i = 0; i < lmks->datas_.size(); ++i) {
          auto lmk = std::static_pointer_cast<
              xstream::XStreamData<hobot::vision::Landmarks>>(lmks->datas_[i]);
          for (size_t i = 0; i < lmk->value.values.size(); ++i) {
            if (i < 5) {
              // do not plot kps on head
              continue;
            }
            const auto &point = lmk->value.values[i];
            if (point.score >= vo_plot_cfg_.kps_thr) {
              LOGD << "kps thr:" << vo_plot_cfg_.kps_thr
                   << "  score:" << point.score;
              cv::circle(bgr,
                         cv::Point(point.x * x_scale + x_offset,
                                   point.y * y_scale + y_offset),
                         3, d_color.at("kps"), d_thickness.at("kps"));
            }
          }

          auto points = std::static_pointer_cast<
                  xstream::XStreamData<hobot::vision::Landmarks>>
                  (lmks->datas_[i])->value.values;
          for (const auto& kps : points) {
            LOGD << "kps score:" << kps.score;
          }

          if (points[15].score >= vo_plot_cfg_.kps_thr &&
              points[13].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[15].x * x_scale + x_offset,
                               points[15].y * y_scale + y_offset),
                     cv::Point(points[13].x * x_scale + x_offset,
                               points[13].y * y_scale + y_offset),
                     CV_RGB(255, 0, 0), d_thickness.at("kps_line"));

          if (points[13].score >= vo_plot_cfg_.kps_thr &&
              points[11].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[13].x * x_scale + x_offset,
                               points[13].y * y_scale + y_offset),
                     cv::Point(points[11].x * x_scale + x_offset,
                               points[11].y * y_scale + y_offset),
                     CV_RGB(255, 85, 0), d_thickness.at("kps_line"));

          if (points[16].score >= vo_plot_cfg_.kps_thr &&
              points[14].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[16].x * x_scale + x_offset,
                               points[16].y * y_scale + y_offset),
                     cv::Point(points[14].x * x_scale + x_offset,
                               points[14].y * y_scale + y_offset),
                     CV_RGB(255, 170, 0), d_thickness.at("kps_line"));

          if (points[14].score >= vo_plot_cfg_.kps_thr &&
              points[12].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[14].x * x_scale + x_offset,
                               points[14].y * y_scale + y_offset),
                     cv::Point(points[12].x * x_scale + x_offset,
                               points[12].y * y_scale + y_offset),
                     CV_RGB(255, 170, 0), d_thickness.at("kps_line"));

          if (points[11].score >= vo_plot_cfg_.kps_thr &&
              points[12].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[11].x * x_scale + x_offset,
                               points[11].y * y_scale + y_offset),
                     cv::Point(points[12].x * x_scale + x_offset,
                               points[12].y * y_scale + y_offset),
                     CV_RGB(170, 255, 0), d_thickness.at("kps_line"));

          if (points[5].score >= vo_plot_cfg_.kps_thr &&
              points[11].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[5].x * x_scale + x_offset,
                               points[5].y * y_scale + y_offset),
                     cv::Point(points[11].x * x_scale + x_offset,
                               points[11].y * y_scale + y_offset),
                     CV_RGB(85, 255, 0), d_thickness.at("kps_line"));

          if (points[6].score >= vo_plot_cfg_.kps_thr &&
              points[12].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[6].x * x_scale + x_offset,
                               points[6].y * y_scale + y_offset),
                     cv::Point(points[12].x * x_scale + x_offset,
                               points[12].y * y_scale + y_offset),
                     CV_RGB(0, 255, 0), d_thickness.at("kps_line"));

          if (points[5].score >= vo_plot_cfg_.kps_thr &&
              points[6].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[5].x * x_scale + x_offset,
                               points[5].y * y_scale + y_offset),
                     cv::Point(points[6].x * x_scale + x_offset,
                               points[6].y * y_scale + y_offset),
                     CV_RGB(0, 255, 85), d_thickness.at("kps_line"));

          if (points[5].score >= vo_plot_cfg_.kps_thr &&
              points[7].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[5].x * x_scale + x_offset,
                               points[5].y * y_scale + y_offset),
                     cv::Point(points[7].x * x_scale + x_offset,
                               points[7].y * y_scale + y_offset),
                     CV_RGB(0, 255, 170), d_thickness.at("kps_line"));

          if (points[6].score >= vo_plot_cfg_.kps_thr &&
              points[8].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[6].x * x_scale + x_offset,
                               points[6].y * y_scale + y_offset),
                     cv::Point(points[8].x * x_scale + x_offset,
                               points[8].y * y_scale + y_offset),
                     CV_RGB(0, 255, 255), d_thickness.at("kps_line"));

          if (points[7].score >= vo_plot_cfg_.kps_thr &&
              points[9].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[7].x * x_scale + x_offset,
                               points[7].y * y_scale + y_offset),
                     cv::Point(points[9].x * x_scale + x_offset,
                               points[9].y * y_scale + y_offset),
                     CV_RGB(0, 170, 255), d_thickness.at("kps_line"));

          if (points[8].score >= vo_plot_cfg_.kps_thr &&
              points[10].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[8].x * x_scale + x_offset,
                               points[8].y * y_scale + y_offset),
                     cv::Point(points[10].x * x_scale + x_offset,
                               points[10].y * y_scale + y_offset),
                     CV_RGB(0, 85, 255), d_thickness.at("kps_line"));

          // do not plot kps line on head
#if 0
          if (points[1].score >= vo_plot_cfg_.kps_thr &&
              points[2].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[1].x * x_scale + x_offset,
                               points[1].y * y_scale + y_offset),
                     cv::Point(points[2].x * x_scale + x_offset,
                               points[2].y * y_scale + y_offset),
                     CV_RGB(0, 0, 255), d_thickness.at("kps_line"));

          if (points[0].score >= vo_plot_cfg_.kps_thr &&
              points[1].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[0].x * x_scale + x_offset,
                               points[0].y * y_scale + y_offset),
                     cv::Point(points[1].x * x_scale + x_offset,
                               points[1].y * y_scale + y_offset),
                     CV_RGB(85, 0, 255), d_thickness.at("kps_line"));

          if (points[0].score >= vo_plot_cfg_.kps_thr &&
              points[2].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[0].x * x_scale + x_offset,
                               points[0].y * y_scale + y_offset),
                     cv::Point(points[2].x * x_scale + x_offset,
                               points[2].y * y_scale + y_offset),
                     CV_RGB(170, 0, 255), d_thickness.at("kps_line"));

          if (points[1].score >= vo_plot_cfg_.kps_thr &&
              points[3].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[1].x * x_scale + x_offset,
                               points[1].y * y_scale + y_offset),
                     cv::Point(points[3].x * x_scale + x_offset,
                               points[3].y * y_scale + y_offset),
                     CV_RGB(255, 0, 255), d_thickness.at("kps_line"));

          if (points[2].score >= vo_plot_cfg_.kps_thr &&
              points[4].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[2].x * x_scale + x_offset,
                               points[2].y * y_scale + y_offset),
                     cv::Point(points[4].x * x_scale + x_offset,
                               points[4].y * y_scale + y_offset),
                     CV_RGB(255, 0, 170), d_thickness.at("kps_line"));

          if (points[3].score >= vo_plot_cfg_.kps_thr &&
              points[5].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[3].x * x_scale + x_offset,
                               points[3].y * y_scale + y_offset),
                     cv::Point(points[5].x * x_scale + x_offset,
                               points[5].y * y_scale + y_offset),
                     CV_RGB(255, 0, 85), d_thickness.at("kps_line"));

          if (points[4].score >= vo_plot_cfg_.kps_thr &&
              points[6].score >= vo_plot_cfg_.kps_thr)
            cv::line(bgr,
                     cv::Point(points[4].x * x_scale + x_offset,
                               points[4].y * y_scale + y_offset),
                     cv::Point(points[6].x * x_scale + x_offset,
                               points[6].y * y_scale + y_offset),
                     CV_RGB(0, 0, 255), d_thickness.at("kps_line"));
#endif
        }
      }
    }
  }

  if (0 == vot_data->channel) {
    for (const auto veh : vot_data->vehicle_infos) {
      if (veh.box.score < vo_plot_cfg_.kps_thr) {
        continue;
      }
      cv::rectangle(bgr, cv::Point(veh.box.x1 * x_scale, veh.box.y1 * y_scale),
                    cv::Point(veh.box.x2 * x_scale, veh.box.y2 * y_scale),
                    d_color.at("vehicle"), d_thickness.at("vehicle"));

      if (veh.plate_info.box.x2 - veh.plate_info.box.x1 > 0) {
        LOGI << "plate w:" << veh.plate_info.box.x2 - veh.plate_info.box.x1
             << "  h:" << veh.plate_info.box.y2 - veh.plate_info.box.y1
             << "  score:" << veh.plate_info.box.score
             << "  num:" << veh.plate_info.plate_num;
      }
      cv::rectangle(bgr,
                    cv::Point(veh.plate_info.box.x1 * x_scale,
                              veh.plate_info.box.y1 * y_scale),
                    cv::Point(veh.plate_info.box.x2 * x_scale,
                              veh.plate_info.box.y2 * y_scale),
                    d_color.at("plate"), d_thickness.at("plate"));

      if (!veh.plate_info.plate_num.empty()) {
        LOGI << "plot plate_num:" << veh.plate_info.plate_num;
        cv::putText(bgr, veh.plate_info.plate_num.substr(3),
                    cv::Point(veh.plate_info.box.x1 * x_scale,
                              veh.plate_info.box.y1 * y_scale),
                    cv::HersheyFonts::FONT_HERSHEY_PLAIN, 1.5,
                    CV_RGB(255, 255, 255), 2);
      }
    }

    // plot fps
//    static int count_fps = 0;
//    static std::string plot_fps {""};
//    count_fps++;
//    static auto start_fps = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double, std::milli> interval_ms =
//            std::chrono::high_resolution_clock::now() - start_fps;
//    if (interval_ms.count() >= 1000) {
//      LOGI << "plot fps " << count_fps;
//      plot_fps = "fps " + std::to_string(count_fps);
//      count_fps = 0;
//      start_fps = std::chrono::high_resolution_clock::now();
//    }
//    if (!plot_fps.empty()) {
//      cv::putText(bgr, plot_fps,
//                  cv::Point(10, 540 - 20),
//                  cv::HersheyFonts::FONT_HERSHEY_PLAIN,
//                  1.5, CV_RGB(255, 0, 255), 2);
//    }
  }

  DrawLogo(&bgr, vot_data->channel);
  return 0;
}

void VotModule::bgr_to_nv12(uint8_t *bgr, uint8_t *buf) {
  int uv_height = image_height_ / 2;
  int uv_width = image_width_ / 2;
  int uv_size = uv_height * uv_width;
  uint8_t *uv_data = buf + (uv_size << 2);
  uint8_t *uv_data_store = bgr + (uv_size << 2) * 3;
  libyuv::RGB24ToI420(bgr, uv_width * 6,
                      buf, uv_width * 2,
                      uv_data_store, uv_width,
                      uv_data_store + uv_size, uv_width,
                      uv_width * 2, uv_height * 2);
  // copy uv data
  for (int i = 0; i < uv_size; ++i) {
    *(uv_data++) = *(uv_data_store + i);
    *(uv_data++) = *(uv_data_store + uv_size + i);
  }
}

void VotModule::bgr_540p_to_nv12(cv::Mat& bgr_mat, char *buf, int channel) {
  int pad_x = 0;
  int pad_y = 0;
  if (0 == channel) {
    pad_x = 0;
    pad_y = 0;
  } else if (1 == channel) {
    pad_x = image_width_;
    pad_y = 0;
  } else if (2 == channel) {
    pad_x = 0;
    pad_y = image_height_;
  } else if (3 == channel) {
    pad_x = image_width_;
    pad_y = image_height_;
  }

  uint8_t *img_i420 = reinterpret_cast<uint8_t *>
  (malloc(image_width_ * image_height_ * 3 / 2));
  bgr_to_nv12(bgr_mat.ptr<uint8_t>(), img_i420);
  for (uint32_t i = 0; i < image_height_; ++i) {
    memcpy(buf + (i + pad_y) * 1920 + pad_x,
           img_i420 + i * image_width_,
           image_width_);
  }
  for (uint32_t i = 0; i < image_height_ / 2; ++i) {
    memcpy(buf + (i + 1080 + pad_y / 2) * 1920 + pad_x,
           img_i420 + image_width_ * image_height_ + i * image_width_,
           image_width_);
  }
  free(img_i420);
  return;
}

void VotModule::bgr_to_nv12_ex(uint8_t *bgr, uint8_t *buf, const uint32_t width,
                               const uint32_t height) {
  int uv_height = height / 2;
  int uv_width = width / 2;
  int uv_size = uv_height * uv_width;
  uint8_t *uv_data = buf + (uv_size << 2);
  uint8_t *uv_data_store = bgr + (uv_size << 2) * 3;
  libyuv::RGB24ToI420(bgr, uv_width * 6, buf, uv_width * 2, uv_data_store,
                      uv_width, uv_data_store + uv_size, uv_width, uv_width * 2,
                      uv_height * 2);
  // copy uv data
  for (int i = 0; i < uv_size; ++i) {
    *(uv_data++) = *(uv_data_store + i);
    *(uv_data++) = *(uv_data_store + uv_size + i);
  }
}

void VotModule::bgr_540p_to_nv12_ex(const uint32_t src_width,
                                    const uint32_t src_height, cv::Mat &bgr_mat,
                                    char *buf, int channel, bool &resize) {
  int pad_x = 0;
  int pad_y = 0;
  uint32_t image_width = 0;
  uint32_t image_height = 0;
  DisplayInfo::computeXYPossition(display_mode_, channel_num_, channel, pad_x,
                                  pad_y);
  DisplayInfo::computeResolution(display_mode_, channel_num_, channel,
                                 image_width, image_height);
  uint8_t *img_i420 =
      reinterpret_cast<uint8_t *>(malloc(image_width * image_height * 3 / 2));
  if (src_width != image_width || src_height != image_height) {
    LOGW << "display mode:" << display_mode_
         << " src resolution and dst resolution not equal, need to resize!!!";
    cv::resize(bgr_mat, bgr_mat, cv::Size(image_width, image_height));
    resize = true;
  }

  bgr_to_nv12_ex(bgr_mat.ptr<uint8_t>(), img_i420, image_width, image_height);
  for (uint32_t i = 0; i < image_height; ++i) {
    memcpy(buf + (i + pad_y) * 1920 + pad_x, img_i420 + i * image_width,
           image_width);
  }
  for (uint32_t i = 0; i < image_height / 2; ++i) {
    memcpy(buf + (i + 1080 + pad_y / 2) * 1920 + pad_x,
           img_i420 + image_width * image_height + i * image_width,
           image_width);
  }

  free(img_i420);
  return;
}

int VotModule::Input(void *data, const xstream::OutputDataPtr &xstream_out) {
  int ret = 0;
  VotData *vot_data = static_cast<VotData *>(data);
#ifdef PRINTF_USE_TIME
  struct timeval start_time, finish_time, totalstart, totoalend;
  double timeuse = 0;
  if (vot_data->channel == 0) {
    gettimeofday(&start_time, NULL);
    gettimeofday(&totalstart, NULL);
  }
#endif

  uint32_t image_width = vot_data->width;
  uint32_t image_height = vot_data->height;
  auto image_data_size_ = image_width * image_height * 3;
  bool resize = false;
  uint8_t *bgr_buf = new uint8_t[image_data_size_ / 2 + image_data_size_ * 2];
  cv::Mat bgr(image_height, image_width, CV_8UC3, bgr_buf);
  PlotSmartData(bgr, true, false, false, false, false, vot_data, xstream_out);
  bgr_540p_to_nv12_ex(vot_data->width, vot_data->height, bgr, buffer_,
                      vot_data->channel, resize);

  if (!resize) delete[] bgr_buf;
#ifdef PRINTF_USE_TIME
  if (vot_data->channel == 0) {
    gettimeofday(&finish_time, NULL);
    timeuse = finish_time.tv_sec - start_time.tv_sec +
              (finish_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("draw smart data plot data Use Time:%f\n", timeuse);

    gettimeofday(&start_time, NULL);
  }
#endif
  VOT_FRAME_INFO_S stFrame = {};
  stFrame.addr = buffer_;
  stFrame.size = 1920 * 1080 * 3 / 2;
  ret = HB_VOT_SendFrame(0, 0, &stFrame, -1);
#ifdef PRINTF_USE_TIME
  if (vot_data->channel == 0) {
    gettimeofday(&finish_time, NULL);
    gettimeofday(&totoalend, NULL);
    timeuse = finish_time.tv_sec - start_time.tv_sec +
              (finish_time.tv_usec - start_time.tv_usec) / 1000000.0;
    printf("draw smart data send data to vo Use Time:%f\n", timeuse);
    timeuse = totoalend.tv_sec - totalstart.tv_sec +
              (totoalend.tv_usec - totalstart.tv_usec) / 1000000.0;
    printf("draw smart data plot and send total Use Time:%f\n", timeuse);
  }
#endif
  return ret;
}

int VotModule::Start() {
  return HB_VOT_EnableChn(0, 0);
}

int VotModule::Stop() {
  int ret = 0;
  ret = HB_VOT_DisableChn(0, 0);
  if (ret)
    LOGE << "HB_VOT_DisableChn failed.";

  ret = HB_VOT_DisableVideoLayer(0);
  if (ret)
    LOGE << "HB_VOT_DisableVideoLayer failed.";

  ret = HB_VOT_Disable(0);
  if (ret)
    LOGE << "HB_VOT_Disable failed.";
  return ret;
}

int VotModule::DeInit() {
  free(buffer_);
  return 0;
}

void VotModule::padding_logo(char *data, int pad_width, int pad_height) {
  // padding yuv420_mat to 1080P data
  // the start padding position is (pad_x, pad_y)
  // yuv420_mat size is mat_width and mat_height
  auto padding = [this, &data, &pad_width, &pad_height]
          (const cv::Mat& yuv420_mat,
           const int& pad_x,
           const int& pad_y,
           const int& mat_width,
           const int& mat_height) {
      uint32_t in_offset = 0;
      uint32_t out_offset = pad_y * pad_width + pad_x;
      // padding Y
      for (auto idx = 0; idx < mat_height; idx++) {
        memcpy(&data[out_offset], &yuv420_mat.data[in_offset], mat_width);
        in_offset += mat_width;
        out_offset += pad_width;
      }
      // padding UV
      // has UV data
      int uv_height = mat_height/2;
      int uv_width = mat_width/2;
      int uv_stride = uv_height * uv_width;
      out_offset = pad_width * pad_height + pad_y/2 * pad_width + pad_x;
      uint8_t* uv_ptr = yuv420_mat.data + in_offset;
      for (int i = 0; i < uv_height; i++) {
        for (int j = 0; j < uv_width; j++) {
          data[out_offset++] = *(uv_ptr + i * uv_width + j);
          data[out_offset++] = *(uv_ptr + uv_stride + i * uv_width + j);
        }
        out_offset = pad_width * pad_height
                     + (pad_y/2 + i) * pad_width + pad_x;
      }
  };

  // top
  if (!logo_img_cache_.top_yuv_mat_.empty()) {
    padding(logo_img_cache_.top_yuv_mat_, 0, 0,
            logo_img_cache_.top_image_width_,
            logo_img_cache_.top_image_height_);
  } else {
    LOGI << "no top logo";
  }

  // bottom
  if (!logo_img_cache_.bottom_yuv_mat_.empty()) {
    padding(logo_img_cache_.bottom_yuv_mat_,
            (pad_width - logo_img_cache_.bottom_image_width_) / 2,
            pad_height - logo_img_cache_.bottom_image_height_,
            logo_img_cache_.bottom_image_width_,
            logo_img_cache_.bottom_image_height_);
  } else {
    LOGI << "no bottom logo";
  }
  return;
}

int VotModule::ParseBottomLogoImg(const std::string& file_name_bottom_left,
                                  const std::string& file_name_bottom_rigth) {
  logo_img_cache_.bottom_bgr_mat_left_ = cv::imread(file_name_bottom_left,
                                                    CV_LOAD_IMAGE_UNCHANGED);
  if (!logo_img_cache_.bottom_bgr_mat_left_.data) {
    LOGE << "Failed to call imread for " << file_name_bottom_left;
    return -1;
  }
  logo_img_cache_.bottom_bgr_mat_right_ = cv::imread(file_name_bottom_rigth,
                                                     CV_LOAD_IMAGE_UNCHANGED);
  if (!logo_img_cache_.bottom_bgr_mat_right_.data) {
    LOGE << "Failed to call imread for " << file_name_bottom_rigth;
    return -1;
  }

  return 0;
}


int VotModule::ParseLogoImg(const std::string& file_name_top,
                            const std::string& file_name_bottom,
                            int pad_width, int pad_height) {
  auto bgr_mat_top = cv::imread(file_name_top);
  if (!bgr_mat_top.data) {
    LOGE << "Failed to call imread for " << file_name_top;
    return -1;
  }
  auto bgr_mat_bottom = cv::imread(file_name_bottom);
  if (!bgr_mat_bottom.data) {
    LOGE << "Failed to call imread for " << file_name_bottom;
    return -1;
  }

  auto ori_width = bgr_mat_top.cols;
  auto ori_height = bgr_mat_top.rows;
  if (ori_width > pad_width || ori_height > pad_height) {
    auto aspect_ratio = ori_width / ori_height;
    auto dst_ratio = static_cast<float>(pad_width) / ori_height;
    uint32_t resized_width = -1;
    uint32_t resized_height = -1;
    // 等比缩放
    if (aspect_ratio >= dst_ratio) {
      resized_width = pad_width;
      resized_height =
          static_cast<uint32_t>(ori_height * pad_width / ori_width);
    } else {
      resized_width =
          static_cast<uint32_t>(ori_width * pad_height / ori_height);
      resized_height = pad_height;
    }
    // mat should allign with 2
    cv::resize(bgr_mat_top, bgr_mat_top,
               cv::Size(resized_width / 2 * 2, resized_height / 2 * 2));
  }
  logo_img_cache_.top_bgr_mat_ = bgr_mat_top;
  if (display_mode_ == 0) {
    if (channel_num_ == 1) {
      logo_img_cache_.top_bgr_mat_left_ = bgr_mat_top;
    } else if (channel_num_ > 1 && channel_num_ <= 4) {
      logo_img_cache_.top_bgr_mat_left_ =
          bgr_mat_top(cv::Rect(0, 0, 960, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_right_ =
          bgr_mat_top(cv::Rect(960, 0, 960, bgr_mat_top.rows));
    } else {
      logo_img_cache_.top_bgr_mat_left_ =
          bgr_mat_top(cv::Rect(0, 0, 640, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_mid_ =
          bgr_mat_top(cv::Rect(640, 0, 640, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_right_ =
          bgr_mat_top(cv::Rect(1280, 0, 640, bgr_mat_top.rows));
    }
  } else {  // 9窗口
    logo_img_cache_.top_bgr_mat_left_ =
        bgr_mat_top(cv::Rect(0, 0, 640, bgr_mat_top.rows));
    logo_img_cache_.top_bgr_mat_mid_ =
        bgr_mat_top(cv::Rect(640, 0, 640, bgr_mat_top.rows));
    logo_img_cache_.top_bgr_mat_right_ =
        bgr_mat_top(cv::Rect(1280, 0, 640, bgr_mat_top.rows));
  }

  logo_img_cache_.top_image_width_ = bgr_mat_top.cols;
  logo_img_cache_.top_image_height_ = bgr_mat_top.rows;
  cv::cvtColor(bgr_mat_top, logo_img_cache_.top_yuv_mat_,
               cv::COLOR_BGR2YUV_I420);

  if (display_mode_ == 0) {
    if (channel_num_ == 1) {
      logo_img_cache_.top_bgr_mat_left_ = bgr_mat_top;
    } else if (channel_num_ > 1 && channel_num_ <= 4) {
      logo_img_cache_.top_bgr_mat_left_ =
          bgr_mat_top(cv::Rect(0, 0, 960, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_right_ =
          bgr_mat_top(cv::Rect(960, 0, 960, bgr_mat_top.rows));
    } else {
      logo_img_cache_.top_bgr_mat_left_ =
          bgr_mat_top(cv::Rect(0, 0, 640, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_mid_ =
          bgr_mat_top(cv::Rect(640, 0, 640, bgr_mat_top.rows));
      logo_img_cache_.top_bgr_mat_right_ =
          bgr_mat_top(cv::Rect(1280, 0, 640, bgr_mat_top.rows));
    }
  } else {  // 9窗口
    logo_img_cache_.top_bgr_mat_left_ =
        bgr_mat_top(cv::Rect(0, 0, 640, bgr_mat_top.rows));
    logo_img_cache_.top_bgr_mat_mid_ =
        bgr_mat_top(cv::Rect(640, 0, 640, bgr_mat_top.rows));
    logo_img_cache_.top_bgr_mat_right_ =
        bgr_mat_top(cv::Rect(1280, 0, 640, bgr_mat_top.rows));
  }

  // crop bottom logo from trapezium to rectangle
  int w_offset = 64;
  cv::Rect rect(w_offset, 0, bgr_mat_bottom.cols - w_offset * 2,
                bgr_mat_bottom.rows);
  cv::Mat new_bottom = bgr_mat_bottom(rect);
  logo_img_cache_.bottom_image_width_ = new_bottom.cols;
  logo_img_cache_.bottom_image_height_ = new_bottom.rows;
  cv::cvtColor(new_bottom, logo_img_cache_.bottom_yuv_mat_,
               cv::COLOR_BGR2YUV_I420);
  cv::resize(new_bottom, logo_img_cache_.bottom_bgr_mat_,
             cv::Size(new_bottom.cols / 8 * 2, new_bottom.rows / 8 * 2));

  return 0;
}

int VotModule::PlotFont(char *y_mat, const char* font_buf,
                        int x0, int y0, int bg_width, int bg_height) {
  if (!y_mat || !font_buf) {
    LOGE << "plot font fail! mat/font invalid";
    return -1;
  }
  static int32_t tagY = 226;
  //操作屏幕
  for (int index = 0; index < w_font_ * h_font_ / 8; index++) {
    for (int i = 7; i >= 0; i--) {
      if (font_buf[index] & (1 << i)) {
        int x = 8 * (index % (w_font_ / 8)) + 7 - i + x0;
        int y = index / (w_font_ / 8) + y0;
        if (x >= bg_width || y >= bg_height) {
          continue;
        }

        *(y_mat + bg_width * y + x) = tagY;
        *(y_mat + y*bg_width + x + 1) = tagY;
        *(y_mat + (y + 1)*bg_width + x) = tagY;
        *(y_mat + (y + 1)*bg_width + x + 1) = tagY;
      }
    }
  }
  return 0;
}

void VotModule::DrawLogo(cv::Mat *bgr, const int channel) {
  if (display_mode_ == 0 && channel_num_ == 1) {
    if (logo_img_cache_.top_bgr_mat_.data) {
      Drawlogo(logo_img_cache_.top_bgr_mat_, bgr, 0);
    }
    // bottom
    if (logo_img_cache_.bottom_bgr_mat_left_.data) {
      Drawlogo(logo_img_cache_.top_bgr_mat_left_, bgr, 1, 0);
    }
  } else if (display_mode_ == 0 && (channel_num_ > 1 && channel_num_ <= 4)) {
    if (logo_img_cache_.top_bgr_mat_left_.data &&
        logo_img_cache_.top_bgr_mat_right_.data) {
      if (0 == channel) {
        Drawlogo(logo_img_cache_.top_bgr_mat_left_, bgr, 0);
      } else if (1 == channel) {
        Drawlogo(logo_img_cache_.top_bgr_mat_right_, bgr, 0);
      }
    }

    // bottom
    if (logo_img_cache_.bottom_bgr_mat_left_.data &&
        logo_img_cache_.bottom_bgr_mat_right_.data) {
      if (2 == channel) {
        Drawlogo(logo_img_cache_.bottom_bgr_mat_left_, bgr, 1, 0);
      } else if (3 == channel) {
        Drawlogo(logo_img_cache_.bottom_bgr_mat_right_, bgr, 1, 1);
      }
    }
  } else {
    if (logo_img_cache_.top_bgr_mat_left_.data &&
        logo_img_cache_.top_bgr_mat_mid_.data &&
        logo_img_cache_.top_bgr_mat_right_.data) {
      if (0 == channel) {
        Drawlogo(logo_img_cache_.top_bgr_mat_left_, bgr, 0);
      } else if (1 == channel) {
        Drawlogo(logo_img_cache_.top_bgr_mat_mid_, bgr, 0);
      } else if (2 == channel) {
        Drawlogo(logo_img_cache_.top_bgr_mat_right_, bgr, 0);
      }
    }

    // bottom
    if (logo_img_cache_.bottom_bgr_mat_left_.data &&
        logo_img_cache_.bottom_bgr_mat_right_.data) {
      if (6 == channel) {
        Drawlogo(logo_img_cache_.bottom_bgr_mat_left_, bgr, 1, 0);
      } else if (7 == channel) {
        Drawlogo(logo_img_cache_.bottom_bgr_mat_mid_, bgr, 1, 1);
      } else if (8 == channel) {
        Drawlogo(logo_img_cache_.bottom_bgr_mat_right_, bgr, 1, 1);
      }
    }
  }
}

int VotModule::Drawlogo(const cv::Mat &logo, cv::Mat *bgr, int position,
                        int left_right) {
  if (logo.data == nullptr || !bgr) {
    return -1;
  }

  cv::Point point_x;
  cv::Point point_y;
  if (position == 0) {  //  top
    point_x.x = 0;
    point_x.y = 0;
  } else if (position == 1) {  //  bottom_middle
    if (0 == left_right) {
      if (display_mode_ == 0 && channel_num_ == 1) {
        point_x.x = 1920 - logo.cols;
        point_x.y = 1080 - logo.rows;
      } else if (display_mode_ == 0 && channel_num_ > 1 && channel_num_ <= 4) {
        point_x.x = 960 - logo.cols;
        point_x.y = 540 - logo.rows;
      } else {
        point_x.x = 640 - logo.cols;
        point_x.y = 360 - logo.rows;
      }
    } else if (1 == left_right || 2 == left_right) {
      point_x.x = 0;
      if (display_mode_ == 0 && channel_num_ == 1) {
        point_x.y = 1080 - logo.rows;
      } else if (display_mode_ == 0 && channel_num_ > 1 && channel_num_ <= 4) {
        point_x.y = 540 - logo.rows;
      } else {
        point_x.y = 360 - logo.rows;
      }
    }
  }

  point_y.x = logo.cols;
  point_y.y = logo.rows;
  point_y += point_x;
  auto bgr_roi = (*bgr)(cv::Rect(point_x, point_y));
  std::vector<cv::Mat> logo_channels;
  std::vector<cv::Mat> bgr_channels;
  cv::split(logo, logo_channels);
  cv::split(bgr_roi, bgr_channels);
  if (logo_channels.size() == 4) {
    for (int i = 0; i < 3; i++) {
      bgr_channels[i] = bgr_channels[i].mul(
              255.0 - logo_channels[3], 0.003921);
      bgr_channels[i] += logo_channels[i].mul(logo_channels[3], 0.003921);
    }
    merge(bgr_channels, bgr_roi);
  } else {
    logo.copyTo(bgr_roi);
  }
  return 0;
}
}  // namespace vision
}  // namespace horizon
