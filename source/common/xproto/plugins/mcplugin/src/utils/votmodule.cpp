/*
 * @Description:
 * @Author: xx@horizon.ai
 * @Date: 2020-06-22 16:17:25
 * @Copyright 2017~2019 Horizon Robotics, Inc.
 */

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fstream>
#include "hobotlog/hobotlog.hpp"
#include "opencv2/opencv.hpp"
#include "utils/votmodule.h"

#include "xproto_msgtype/protobuf/x3.pb.h"
namespace horizon {
namespace vision {

static int fbfd;
char *fb = NULL;
int hb_vot_init(void)
{
//  int ret = 0;
//  VOT_VIDEO_LAYER_ATTR_S stLayerAttr;
//  VOT_CHN_ATTR_S stChnAttr;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;

  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device1.\n");
    HOBOT_CHECK(0);
  }
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
    HOBOT_CHECK(0);
  }

  /* Get variable screen information */
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
    HOBOT_CHECK(0);
  }
  printf("vinfo.xres=%d\n", vinfo.xres);
  printf("vinfo.yres=%d\n", vinfo.yres);
  printf("vinfo.bits_per_bits=%d\n", vinfo.bits_per_pixel);
  printf("vinfo.xoffset=%d\n", vinfo.xoffset);
  printf("vinfo.yoffset=%d\n", vinfo.yoffset);
  printf("finfo.line_length=%d\n", finfo.line_length);
  printf("finfo.left_margin = %d\n", vinfo.left_margin);
  printf("finfo.right_margin = %d\n", vinfo.right_margin);
  printf("finfo.upper_margin = %d\n", vinfo.upper_margin);
  printf("finfo.lower_margin = %d\n", vinfo.lower_margin);
  printf("finfo.hsync_len = %d\n", vinfo.hsync_len);
  printf("finfo.vsync_len = %d\n", vinfo.vsync_len);
  fb = (char *)mmap(0, 1920 * 1080 * 4, PROT_READ | PROT_WRITE,  // NOLINT
                    MAP_SHARED, fbfd, 0);
  if (fb == (void*)(-1)) {  // NOLINT
    printf("Error: failed to map framebuffer device to memory.\n");
    HOBOT_CHECK(0);
  }
  memset(fb, 0x0, 1920 * 1080 * 4);
  return 0;
}

VotModule::VotModule():group_id_(-1), stop_(true) {
  Init();
//  hb_vot_init();
}

VotModule::~VotModule() {
  Stop();
  if (queue_.size() > 0) {
    queue_.clear();
  }
  free(buffer_);
  DeInit();
}

int VotModule::Start() {
  if (!stop_) return 0;
  stop_ = false;

  // plot task
  auto plot_func = [this](){
      while (!stop_) {
        std::shared_ptr<VotData_t> vot_data;
        auto is_getitem = in_queue_.try_pop(&vot_data,
                                         std::chrono::milliseconds(1000));
        if (!is_getitem) {
          continue;
        }
        auto sp_buf = PlotImage(vot_data);
        if (sp_buf) {
          queue_.push(sp_buf);
        }
      }
    };
  for (uint32_t idx = 0; idx < plot_task_num_; idx++) {
    plot_tasks_.emplace_back(std::make_shared<std::thread>(plot_func));
  }

  // send vot display task
  auto send_vot_func = [this](){
      while (!stop_) {
        std::shared_ptr<char> sp_buf;
        auto is_getitem = queue_.try_pop(&sp_buf,
            std::chrono::milliseconds(1000));
        if (!buffer_ || !is_getitem) continue;
        memcpy(buffer_, sp_buf.get(), image_data_size_);

        static VOT_FRAME_INFO_S stFrame {buffer_, image_data_size_};
        HB_VOT_SendFrame(0, channel_, &stFrame, -1);
      }
  };
  send_vot_task_ = std::make_shared<std::thread>(send_vot_func);

//  display_task_ = std::make_shared<std::thread>([this]{
//    while (!stop_) {
//      static VOT_FRAME_INFO_S stFrame {buffer_, image_data_size_};
//      HB_VOT_SendFrame(0, channel_, &stFrame, -1);
//      std::this_thread::sleep_for(std::chrono::milliseconds(1));
//    }
//  });
  return 0;
}

int VotModule::Init() {
  int ret = 0;
  image_height_ = 1080;
  image_width_ = 1920;
  image_data_size_ = image_width_ * image_height_ * 3 / 2;

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
  stLayerAttr.stImageSize.u32Width  = 1920;
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
  ret = HB_VOT_SetChnAttr(0, channel_, &stChnAttr);
  printf("HB_VOT_SetChnAttr 0: %d\n", ret);

  cropAttrs.u32Width = stChnAttr.u32DstWidth;
  cropAttrs.u32Height = stChnAttr.u32DstHeight;
  ret = HB_VOT_SetChnCrop(0, channel_, &cropAttrs);
  printf("HB_VOT_EnableChn: %d\n", ret);

  ret = HB_VOT_EnableChn(0, channel_);
  printf("HB_VOT_EnableChn: %d\n", ret);

  buffer_ = (char *)malloc(image_data_size_);// NOLINT
  return ret;
}

void VotModule::bgr_to_nv12(uint8_t *bgr, char *buf) {
  cv::Mat bgr_mat(image_height_, image_width_, CV_8UC3, bgr);
  cv::Mat yuv_mat;
  cv::cvtColor(bgr_mat, yuv_mat, cv::COLOR_BGR2YUV_I420);

  int uv_height = image_height_ / 2;
  int uv_width = image_width_ / 2;
  // copy y data
  int y_size = uv_height * uv_width * 4;

  uint8_t *yuv = yuv_mat.ptr<uint8_t>();
  memcpy(buf, yuv, y_size);

  // copy uv data
  int uv_stride = uv_width * uv_height;
  char *uv_data = buf + y_size;
  for (int i = 0; i < uv_stride; ++i) {
    *(uv_data++) = *(yuv + y_size + i);
    *(uv_data++) = *(yuv + y_size + +uv_stride + i);
  }

//  {
//    auto start = std::chrono::high_resolution_clock::now();
//    auto end = std::chrono::high_resolution_clock::now();
//    std::chrono::duration<double, std::milli> cost = end - start;
//    LOGD << "bgr_to_nv12 cost ms " << cost.count();
//  }
}

std::shared_ptr<char>
VotModule::PlotImage(const std::shared_ptr<VotData_t>& vot_data) {
  HOBOT_CHECK(vot_data->sp_img_);
  HOBOT_CHECK(vot_data->sp_smart_pb_);

  if (!vot_data->sp_img_) {
    LOGE << "invalid image in smart frame";
    return nullptr;
  }

  LOGI << "plot frame_id:" << vot_data->sp_img_->frame_id;
  const auto& image = vot_data->sp_img_->image;
  const auto& height = image.height;
  const auto& width = image.width;
  if (height <= 0 || width <= 0 ||
          image.data_size != height * width * 3 / 2) {
    LOGE << "pyrmid: " << width << "x" << height;
    HOBOT_CHECK(0);
    return nullptr;
  }
  auto *img_addr = image.data;
  auto start = std::chrono::high_resolution_clock::now();

//  {
//    static int count = 0;
//    std::ofstream ofs("pym_vot_" + std::to_string(count++) + ".yuv");
//    ofs.write(reinterpret_cast<char*>(img_addr), image.data_size);
//  }

  cv::Mat bgr;
  cv::cvtColor(cv::Mat(height * 3 / 2, width, CV_8UC1, img_addr),
               bgr, CV_YUV2BGR_NV12);
  {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> cost = end - start;
    LOGD << "cvt bgr cost ms " << cost.count();
    start = std::chrono::high_resolution_clock::now();
  }

  if (image_width_ != image.width ||
          image_height_ != image.height) {
    HOBOT_CHECK(image_width_ / image.width ==
                image_height_ / image.height);
    float scale_factor = static_cast<float>(image_width_) /
                         static_cast<float>(image.width);
    cv::resize(bgr, bgr, cv::Size(), scale_factor, scale_factor);
  }

  const static std::map<std::string, decltype(CV_RGB(255, 0, 0))> d_color = // NOLINT
          {{"id", CV_RGB(255, 0, 0)},
           {"face", CV_RGB(255, 0, 0)},
           {"head", CV_RGB(0, 255, 0)},
           {"body", CV_RGB(255, 255, 0)},
           {"lmk", CV_RGB(255, 0, 0)},
           {"kps", CV_RGB(0, 0, 255)},
           {"fps", CV_RGB(255, 0, 255)}
          };

  // plot fps
  static int count_fps = 0;
  static std::string plot_fps {""};
  count_fps++;
  static auto start_fps = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> interval_ms =
          std::chrono::high_resolution_clock::now() - start_fps;
  if (interval_ms.count() >= 1000) {
    LOGI << "plot fps " << count_fps;
    plot_fps = "fps " + std::to_string(count_fps);
    count_fps = 0;
    start_fps = std::chrono::high_resolution_clock::now();
  }
  if (!plot_fps.empty()) {
    cv::putText(bgr, plot_fps,
                cv::Point(10, 20),
                cv::HersheyFonts::FONT_HERSHEY_PLAIN,
                1.5, d_color.at("fps"), 2);
  }

  std::string plot_frame_id = "frame id "
                              + std::to_string(vot_data->sp_img_->frame_id);
  cv::putText(bgr, plot_frame_id,
              cv::Point(10, 60),
              cv::HersheyFonts::FONT_HERSHEY_PLAIN,
              1.5, d_color.at("fps"), 2);

//  {
//    static int count = 0;
//    std::string pic_name{"plot_" + std::to_string(count++) + ".jpg"};
//    LOGD << "pic_name:" << pic_name;
//    cv::imwrite(pic_name, bgr);
//  }

  // plot smart data from pb
  x3::MessagePack pack;
  x3::FrameMessage frame;
//  HOBOT_CHECK(vot_data->sp_smart_pb_);
//  HOBOT_CHECK(pack.ParseFromString(*vot_data->sp_smart_pb_));
//  HOBOT_CHECK(frame.ParseFromString(pack.content_()));
//  HOBOT_CHECK(frame.has_smart_msg_());

  if (vot_data->sp_smart_pb_ &&
          pack.ParseFromString(*vot_data->sp_smart_pb_) &&
          frame.ParseFromString(pack.content_()) &&
          frame.has_smart_msg_()) {
    LOGD << "frame_id " << vot_data->sp_img_->frame_id;
    for (int idx_tar = 0; idx_tar < frame.smart_msg_().targets__size();
         idx_tar++) {
      const auto& target = frame.smart_msg_().targets_(idx_tar);
      HorizonVisionSmartData s_data;
      s_data.face = NULL;
      s_data.body = NULL;
      s_data.track_id = target.track_id_();
      HorizonVisionFaceSmartData face;
      HorizonVisionBodySmartData body;
//      HorizonVisionLandmarks landmarks;
      std::stringstream ss_attr;

      HorizonVisionBBox face_rect;
      HorizonVisionBBox head_rect;
      bool has_face = false;
      bool has_head = false;

      LOGD << "track_id " << s_data.track_id;

      if (target.boxes__size() > 0) {
        // box
        for (int idx = 0; idx < target.boxes__size(); idx++) {
          const auto& box = target.boxes_(idx);
          LOGD << "box:" << box.type_()
               << " x1:" << box.top_left_().x_()
               << " y1:" << box.top_left_().y_()
               << " x2:" << box.bottom_right_().x_()
               << " y2:" << box.bottom_right_().y_();

          HorizonVisionBBox rect;
          rect.x1 = box.top_left_().x_();
          rect.y1 = box.top_left_().y_();
          rect.x2 = box.bottom_right_().x_();
          rect.y2 = box.bottom_right_().y_();
          rect.score = box.score();
          if ("face" == box.type_()) {
            has_face = true;
            face_rect = rect;
          } else if ("head" == box.type_()) {
            has_head = true;
            head_rect = rect;
          } else if ("body" == box.type_()) {
            body.body_rect = rect;
            s_data.body = &body;
          }
        }
        if (has_face || has_head) {
          s_data.face = &face;
          if (has_face) {
            face.face_rect = face_rect;
          }
          if (has_head) {
            face.head_rect = head_rect;
          }
        }

        // attr
        for (int idx = 0; idx < target.attributes__size(); idx++) {
          const auto attr = target.attributes_(idx);
          LOGD << "attr type:" << attr.type_()
               << " val:" << attr.value_()
               << " score:" << target.attributes_(idx).score_();
          ss_attr << " " << attr.type_() << ":" << attr.value_();
        }

        // points
        for (int idx = 0; idx < target.points__size(); idx++) {
          const auto point = target.points_(idx);
          LOGD << "point type:" << point.type_();
          if (point.type_() == "lmk_106pts") {
            for (int lmk_idx = 0; lmk_idx < point.points__size();
                 lmk_idx++) {
              cv::circle(bgr,
                         cv::Point(point.points_(lmk_idx).x_(),
                                   point.points_(lmk_idx).y_()),
                         1, d_color.at("lmk"), 2);
            }
          }

//          if ("body_landmarks" == point.type_()) {
//            smart_data.kps.num = point.points__size();
//            for (int idx = 0; idx < point.points__size(); idx++) {
//              if (idx >= HORIZON_HERO_LMKS_SIZE_MAX) {
//                LOGE << "";
//                break;
//              }
//              const auto& pt = point.points_(idx);
//              smart_data.kps.x_list[idx] = pt.x_();
//              smart_data.kps.y_list[idx] = pt.y_();
//              smart_data.kps.score_list[idx] = pt.score_();
//            }
//          }
        }
      }

      if (s_data.face) {
        {
          HorizonVisionBBox rect;
          if (has_face) {
            rect = s_data.face->face_rect;
            cv::rectangle(bgr, cv::Point(rect.x1, rect.y1),
                          cv::Point(rect.x2, rect.y2),
                          d_color.at("face"), 2);
          } else {
            rect = s_data.face->head_rect;
          }
          // track id
          std::stringstream id_info;
          id_info << s_data.track_id;
          if (!ss_attr.str().empty()) {
            id_info << "_" << ss_attr.str();
          }
          {
            std::lock_guard<std::mutex> lk(cache_mtx_);
            auto itr = recog_res_cache_.find(s_data.track_id);
            if (itr != recog_res_cache_.end()) {
              std::string new_url{itr->second->img_uri_list};
              if (itr->second->img_uri_list.rfind("/") > 0) {
                new_url = itr->second->img_uri_list.substr(
                        itr->second->img_uri_list.rfind("/") + 1);
              }
              id_info  //  << "_" << itr->second->record_id
                      << "_" << std::setprecision(3) << itr->second->similar
                      << "_" << new_url;
            } else {
              LOGI << "track id " << s_data.track_id
                   << " has no recog res, cache size: "
                   << recog_res_cache_.size();
            }
          }
          cv::putText(bgr, id_info.str(),
                      cv::Point(rect.x1, rect.y1 - 5),
                      cv::HersheyFonts::FONT_HERSHEY_PLAIN,
                      1.5, d_color.at("id"), 2);
        }

        {
          const auto& rect = s_data.face->head_rect;
          cv::rectangle(bgr,
                        cv::Point(rect.x1, rect.y1),
                        cv::Point(rect.x2, rect.y2),
                        d_color.at("head"), 2);
        }

        // face lmk
        if (s_data.face->landmarks) {
          for (size_t lmk_idx = 0; lmk_idx < s_data.face->landmarks->num;
               lmk_idx++) {
            cv::circle(bgr,
                       cv::Point(s_data.face->landmarks->points[lmk_idx].x,
                                 s_data.face->landmarks->points[lmk_idx].y),
                       1, d_color.at("lmk"), 2);
          }
        }
      }
      if (s_data.body) {
        const auto& rect = s_data.body->body_rect;
        cv::rectangle(bgr, cv::Point(rect.x1, rect.y1),
                      cv::Point(rect.x2, rect.y2),
                      d_color.at("body"), 2);

//      if (s_data.body->skeleton) {
//        for (size_t kps_idx = 0; kps_idx < s_data.body->skeleton->num;
//             kps_idx++) {
//          cv::circle(bgr,
//                     cv::Point(s_data.body->skeleton->points[kps_idx].x,
//                               s_data.body->skeleton->points[kps_idx].y),
//                     1, d_color.at("kps"), 2);
//        }
//      }
      }
    }
  } else {
    LOGE << "parse fail";
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> cost = end - start;
  LOGD << "draw jpeg cost ms " << cost.count();
  start = std::chrono::high_resolution_clock::now();

  char *buf_ = new char[image_data_size_];
  bgr_to_nv12(bgr.ptr<uint8_t>(), buf_);
  {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> cost = end - start;
    LOGD << "cvt to nv12 cost ms " << cost.count();
    start = std::chrono::high_resolution_clock::now();
  }

  std::shared_ptr<char> sps(buf_,
                           [](char* buf) {
                               delete []buf;
                           });
  return sps;
}

int VotModule::Input(const std::shared_ptr<VotData_t>& vot_data) {
//  HOBOT_CHECK(fb);
//  memset(fb, 0x0, 1920 * 1080 * 4);
//  return 0;

  if (stop_) return 0;

  HOBOT_CHECK(vot_data);
  if (vot_data->sp_recog_) {
    // cache recog res to display
    if (vot_data->sp_recog_->is_recognize) {
      std::lock_guard<std::mutex> lk(cache_mtx_);
      recog_res_cache_[vot_data->sp_recog_->track_id] = vot_data->sp_recog_;
    }
  }
  if (!vot_data->sp_img_) {
    return 0;
  }

  if (in_queue_.size() < in_queue_len_max_) {
    in_queue_.push(std::move(vot_data));
  } else {
    LOGE << "vot queue is full";
  }

  return 0;
}

int VotModule::Stop() {
  if (stop_)
    return 0;
  stop_ = true;
  if (send_vot_task_ != nullptr) {
    send_vot_task_->join();
    send_vot_task_ = nullptr;
  }
  if (display_task_ != nullptr) {
    display_task_->join();
    display_task_ = nullptr;
  }
  for (const auto& task : plot_tasks_) {
    task->join();
  }
  recog_res_cache_.clear();
  queue_.clear();
  in_queue_.clear();
  plot_tasks_.clear();

  return 0;
}

int VotModule::DeInit() {
  int ret = 0;
  ret = HB_VOT_DisableChn(0, channel_);
  if (ret) printf("HB_VOT_DisableChn failed.\n");

  ret = HB_VOT_DisableVideoLayer(0);
  if (ret) printf("HB_VOT_DisableVideoLayer failed.\n");

  ret = HB_VOT_Disable(0);
  if (ret) printf("HB_VOT_Disable failed.\n");
  return ret;
}
}   //  namespace vision
}   //  namespace horizon
