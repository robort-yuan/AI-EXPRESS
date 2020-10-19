/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-22 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#include "multivioplugin/vioproduce.h"
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

#include "hobotlog/hobotlog.hpp"
#include "opencv2/opencv.hpp"
#ifdef J3_MEDIA_LIB
#include "./hb_cam_interface.h"
#include "./hb_vio_interface.h"
#endif

#include "multivioplugin/viomessage.h"
#include "multivioplugin/util.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

const std::unordered_map<std::string, VioProduce::TSTYPE>
    VioProduce::str2ts_type_ = {{"raw_ts", TSTYPE::RAW_TS},
                                {"frame_id", TSTYPE::FRAME_ID},
                                {"input_coded", TSTYPE::INPUT_CODED}};

std::shared_ptr<VioProduce>
VioProduce::CreateVioProduce(const std::string &data_source) {
  auto config = VioConfig::GetConfig();
  HOBOT_CHECK(config);
  auto json = config->GetJson();
  std::shared_ptr<VioProduce> vio_produce;
  if ("panel_camera" == data_source) {
    LOGI << "begin create panel_camera";
    std::string vio_config = json["vio_cfg_file"]["panel_camera"].asString();
    LOGI << "vio_config path:" << vio_config;
    vio_produce = std::make_shared<PanelCamera>(vio_config);
  }
  vio_produce->cam_type_ = json["cam_type"].asString();
  vio_produce->ts_type_ = str2ts_type_.find(json["ts_type"].asString())->second;
  return vio_produce;
}

void VioProduce::WaitUntilAllDone() {
  LOGD << "consumed_vio_buffers_=" << consumed_vio_buffers_;
  while (consumed_vio_buffers_ > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(50));
  }
}

bool VioProduce::AllocBuffer() {
  LOGV << "AllocBuffer()";
  LOGV << "count: " << consumed_vio_buffers_;
  if (consumed_vio_buffers_ < max_vio_buffer_) {
    consumed_vio_buffers_++;
    LOGV << "alloc buffer success, consumed_vio_buffers_="
         << consumed_vio_buffers_;
    return true;
  }
  return false;
}

void VioProduce::FreeBuffer() {
  LOGV << "FreeBuffer()";
  if (0 == consumed_vio_buffers_)
    return;
  consumed_vio_buffers_--;
  LOGV << "free buffer success, consumed_vio_buffers_="
       << consumed_vio_buffers_;
}

int VioProduce::SetConfig(VioConfig *config, int index) {
  config_ = config;
  index_ = index;
  return kHorizonVisionSuccess;
}

int VioProduce::SetListener(const Listener &callback) {
  push_data_cb_ = callback;
  return kHorizonVisionSuccess;
}

int VioProduce::Finish() {
  if (is_running_) {
    is_running_ = false;
  }
  WaitUntilAllDone();
  return kHorizonVisionSuccess;
}

int VioProduce::Start() {
  auto func = std::bind(&VioProduce::Run, this);
  task_future_ = Executor::GetInstance()->AddTask(func);
  return 0;
}

int VioProduce::Stop() {
  this->Finish();
  LOGD << "wait task to finish";
  task_future_.get();
  LOGI << "vio produce task done";
  return 0;
}

#if defined(J3_MEDIA_LIB)
bool GetPyramidInfo(int pipe_id, VioFeedbackContext *feed_back_context,
                    char *data, int len) {
  if (nullptr == feed_back_context || nullptr == data) {
    return false;
  }
  hb_vio_buffer_t *src_img_info = &(feed_back_context->src_info_);
  pym_buffer_t *pym_buf = &(feed_back_context->pym_img_info_);
  auto ret =
      hb_vio_get_data(pipe_id, HB_VIO_PYM_FEEDBACK_SRC_DATA, src_img_info);
  if (ret < 0) {
    LOGE << "feedback failed, call hb_vio_get_data failed";
    return false;
  }

  // adapter to x3 api, y and uv address is standalone
  // pym only support yuv420sp format
  int y_img_len = len / 3 * 2;
  int uv_img_len = len / 3;
  memcpy(reinterpret_cast<uint8_t *>(src_img_info->img_addr.addr[0]), data,
         y_img_len);
  memcpy(reinterpret_cast<uint8_t *>(src_img_info->img_addr.addr[1]),
         data + y_img_len, uv_img_len);
  ret = hb_vio_run_pym(pipe_id, src_img_info);
  if (ret < 0) {
    LOGE << "feedback failed, call hb_vio_run_pym failed";
    return false;
  }
  ret = hb_vio_get_data(pipe_id, HB_VIO_PYM_DATA, pym_buf);
  if (ret < 0) {
    LOGE << "feedback failed, call hb_vio_get_data failed";
    return false;
  }
  return true;
}

static int DumpPyramidImage(const address_info_t &vio_image,
                            const std::string &path) {
  auto height = vio_image.height;
  auto width = vio_image.width;
  if (height <= 0 || width <= 0) {
    LOGE << "pyrmid: " << width << "x" << height;
    return -1;
  }
  int y_len = width * height;
  int uv_len = y_len / 2;
  int len = y_len + uv_len;
  uint8_t *yuv_data = new uint8_t[len];
  memcpy(yuv_data, vio_image.addr[0], y_len);
  memcpy(yuv_data + y_len, vio_image.addr[1], uv_len);
  cv::Mat nv12(height * 3 / 2, width, CV_8UC1, yuv_data);
  cv::Mat bgr;
  cv::cvtColor(nv12, bgr, CV_YUV2BGR_NV12);
  cv::imwrite(path.c_str(), bgr);
  LOGD << "saved path: " << path;
  delete[] yuv_data;
  return 0;
}

static void DumpPyramidImage(const pym_buffer_t &vio_info, const int pyr_index,
                             const std::string &path) {
  LOGD << "DumpPyramidImage";
  address_info_t addr_info;
  if (pyr_index < 0) {
    LOGF << "param error";
  }
  if (pyr_index % 4 == 0) {
    addr_info = vio_info.pym[pyr_index / 4];
  } else {
    addr_info = vio_info.pym_roi[pyr_index / 4][(pyr_index % 4) - 1];
  }
  DumpPyramidImage(addr_info, path);
}
#endif  // J3_MEDIA_LIB

int VioCamera::ReadTimestamp(void *addr, uint64_t *timestamp) {
  LOGI << "read time stamp";
  uint8_t *addrp = reinterpret_cast<uint8_t *>(addr);
  uint8_t *datap = reinterpret_cast<uint8_t *>(timestamp);
  int i = 0;
  for (i = 15; i >= 0; i--) {
    if (i % 2)
      datap[(15 - i) / 2] |= (addrp[i] & 0x0f);
    else
      datap[(15 - i) / 2] |= ((addrp[i] & 0x0f) << 4);
  }

  return 0;
}

int VioCamera::Run() {
  bool check_timestamp = false;
  bool enable_vio_profile = false;
  bool dump_pyramid_level_env = false;
  auto check_timestamp_str = getenv("check_timestamp");
  if (check_timestamp_str && !strcmp(check_timestamp_str, "ON")) {
    check_timestamp = true;
  }
  auto vio_profile_str = getenv("vio_profile");
  if (vio_profile_str && !strcmp(vio_profile_str, "ON")) {
    enable_vio_profile = true;
  }
  auto dump_pyramid_level_env_str =
      getenv("dump_pyramid_level");  // set pym level
  if (dump_pyramid_level_env_str) {
    dump_pyramid_level_env = true;
  }

  if (is_running_)
    return -1;
  uint64_t frame_id = 0;
  uint64_t last_timestamp = 0;
  is_running_ = true;
  LOGI << "produce run, pipein: " << index_;
  auto start_time = std::chrono::system_clock::now();
  while (is_running_) {
    uint32_t img_num = 1;

    if (cam_type_ == "mono") {
      // need to update by hangjun.yang
      auto *pvio_image = reinterpret_cast<pym_buffer_t *>(
          std::calloc(1, sizeof(pym_buffer_t)));
      auto res = hb_vio_get_data(index_, HB_VIO_PYM_DATA, pvio_image);

      uint64_t img_time = 0;

      if (ts_type_ == TSTYPE::INPUT_CODED && check_timestamp && res == 0 &&
          pvio_image != nullptr) {
        // must chn6, online chn
        ReadTimestamp(reinterpret_cast<uint8_t *>(pvio_image->pym[0].addr[0]),
                      &img_time);
        LOGD << "src img ts:  " << img_time;

        if (pvio_image->pym_img_info.time_stamp !=
            static_cast<uint64_t>(img_time)) {
          LOGF << "timestamp is different!!! "
               << "image info ts: " << pvio_image->pym_img_info.time_stamp;
        }
      } else if (ts_type_ == TSTYPE::FRAME_ID) {
        pvio_image->pym_img_info.time_stamp = pvio_image->pym_img_info.frame_id;
      }
      if (res != 0 || (check_timestamp &&
                       pvio_image->pym_img_info.time_stamp == last_timestamp)) {
        LOGD << "hb_vio_free_pymbuf: " << res;
        hb_vio_free_pymbuf(index_, HB_VIO_PYM_DATA, pvio_image);
        std::free(pvio_image);
        continue;
      }
      if (dump_pyramid_level_env) {
        int dump_pyr_level = std::stoi(dump_pyramid_level_env_str);
        std::string name = "pyr_images/vio_" + std::to_string(index_) + "_" +
                           std::to_string(frame_id) + ".jpg";
        DumpPyramidImage(*pvio_image, dump_pyr_level, name);
      }
      if (check_timestamp && last_timestamp != 0) {
        HOBOT_CHECK(pvio_image->pym_img_info.time_stamp > last_timestamp)
            << pvio_image->pym_img_info.time_stamp << " <= " << last_timestamp;
      }
      LOGD << "pipeid:" << index_
           << ", Vio TimeStamp: " << pvio_image->pym_img_info.time_stamp;
      LOGD << "image width size:" << pvio_image->pym[0].width;
      last_timestamp = pvio_image->pym_img_info.time_stamp;
      if (frame_id % sample_freq_ == 0 && AllocBuffer()) {
        if (!is_running_) {
          LOGD << "stop vio job";
          hb_vio_free_pymbuf(index_, HB_VIO_PYM_DATA, pvio_image);
          std::free(pvio_image);
          FreeBuffer();
          break;
        }

        auto pym_image_frame_ptr = std::make_shared<PymImageFrame>();
        Convert(pvio_image, *pym_image_frame_ptr);
        pym_image_frame_ptr->channel_id = index_;
        pym_image_frame_ptr->frame_id = frame_id;
        std::vector<std::shared_ptr<PymImageFrame>> pym_images;
        pym_images.push_back(pym_image_frame_ptr);

        std::shared_ptr<VioMessage> input(
        new ImageVioMessage(pym_images, img_num), [&](ImageVioMessage *p) {
            if (p) {
              LOGD << "begin delete ImageVioMessage";
              p->FreeImage();
              FreeBuffer();
              delete (p);
            }
              p = nullptr;
            });

        if (enable_vio_profile) {
          input->CreateProfile();
        }
        HOBOT_CHECK(push_data_cb_);
        LOGD << "create image vio message, frame_id = " << frame_id;
        if (push_data_cb_) {
          push_data_cb_(input);
          LOGD << "Push Image message!!!";
        }
      } else {
        LOGD << "NO VIO BUFFER ";
      }
    } else {
      LOGF << "Don't support type: " << cam_type_;
    }
    ++frame_id;
    if (frame_id != 0 && frame_id % 100 == 0) {
      auto curr_time = std::chrono::system_clock::now();
      auto duration_time =
              std::chrono::duration_cast<std::chrono::milliseconds>(
                  curr_time - start_time);
      start_time = std::chrono::system_clock::now();
      LOGW << "channel_id: " << index_
           << ", vio fps: " << 100000 / duration_time.count();
    }
  }
  LOGI << "vio produce run process exit";
  return kHorizonVisionSuccess;
}

std::mutex PanelCamera::vio_mutex_;
bool PanelCamera::vio_init_ = false;

PanelCamera::PanelCamera(const std::string &cfg_file) {
  LOGI << "in PanelCamera constructor, cfg_file: " << cfg_file;
  std::ifstream if_cfg(cfg_file);
  HOBOT_CHECK(if_cfg.is_open()) << "config file load failed!!";
  std::stringstream oss_config;
  oss_config << if_cfg.rdbuf();
  if_cfg.close();
  Json::Value config_jv;
  oss_config >> config_jv;
  cam_type_ = config_jv["type"].asString();
  std::string cam_cfg_file;
  std::string vio_cfg_file;

  if (cam_type_ == "mono") {
    cam_cfg_file = config_jv["mono_cam_cfg_file"].asString();
    vio_cfg_file = config_jv["mono_vio_cfg_file"].asString();
  } else {
    LOGF << "do not support cam type: " << cam_type_;
  }
  camera_index_ = config_jv["cam_index"].asInt();
  LOGI << "cam_cfg_file:" << cam_cfg_file << ", vio_cfg_file:" << vio_cfg_file
       << ", camera index:" << camera_index_;

#ifdef J3_MEDIA_LIB
  std::lock_guard<std::mutex> lk(vio_mutex_);
  if (vio_init_) {
    return;
  }

  auto ret = hb_vio_init(vio_cfg_file.c_str());
  if (ret < 0) {
    LOGF << "vio init failed";
  } else {
    LOGI << "vio init success";
  }
  ret = hb_cam_init(camera_index_, cam_cfg_file.c_str());
  if (ret < 0) {
    hb_vio_deinit();
    LOGF << "cam init failed";
  } else {
    LOGI << "cam init success";
  }

  int cam_num = config_jv["data_source_num"].asInt();
  camera_num_ = cam_num;
  for (int pipe_id = 0; pipe_id < cam_num; ++pipe_id) {
    ret = hb_vio_start_pipeline(pipe_id);
    if (ret < 0) {
      hb_cam_deinit(camera_index_);
      hb_vio_deinit();
      LOGF << "start pipeline " << pipe_id << " failed";
    } else {
      LOGI << "start pipeline " << pipe_id << " success";
    }
  }
  ret = hb_cam_start_all();
  if (ret < 0) {
    hb_cam_stop_all();
    for (int pipe_id = 0; pipe_id < cam_num; ++pipe_id) {
      hb_vio_stop_pipeline(pipe_id);
    }
    hb_cam_deinit(camera_index_);
    hb_vio_deinit();
    LOGF << "cam start all failed";
  } else {
    LOGI << "cam start all success";
  }
  vio_init_ = true;
#endif
}

PanelCamera::~PanelCamera() {
#ifdef J3_MEDIA_LIB
  std::lock_guard<std::mutex> lk(vio_mutex_);
  if (!vio_init_) {
    return;
  }
  hb_cam_stop_all();
  for (int pipe_id = 0; pipe_id < camera_num_; ++pipe_id) {
    hb_vio_stop_pipeline(pipe_id);
  }
  hb_cam_deinit(camera_index_);
  hb_vio_deinit();
  vio_init_ = false;
#endif
}

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
