/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-01 20:38:52
 * @Version: v0.0.1
 * @Brief: smartplugin impl based on xstream.
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-29 05:04:11
 */

#include "smartplugin_box/smartplugin.h"

#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"
#include "smartplugin_box/convert.h"
#include "smartplugin_box/displayinfo.h"
#include "smartplugin_box/runtime_monitor.h"
#include "smartplugin_box/smart_config.h"
#include "votmodule.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/protobuf/x2.pb.h"
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin_multiplebox {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::basic_msgtype::VioMessage;
using ImageFramePtr = std::shared_ptr<hobot::vision::ImageFrame>;
using XStreamImageFramePtr = xstream::XStreamData<ImageFramePtr>;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;

using horizon::iot::smartplugin_multiplebox::Convertor;

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_SMART_MESSAGE)

SmartPlugin::SmartPlugin(const std::string &smart_config_file) {
  smart_config_file_ = smart_config_file;

  LOGI << "smart config file:" << smart_config_file_;
  monitor_.reset(new RuntimeMonitor());
  Json::Value cfg_jv;
  std::ifstream infile(smart_config_file_);
  infile >> cfg_jv;
  config_.reset(new JsonConfigWrapper(cfg_jv));
  ParseConfig();

  running_vot_ = true;
  GetDisplayConfigFromFile(display_config_file_);

  if (running_vot_) {
    vot_module_ = std::make_shared<VotModule>();
  }

  if (running_venc_1080p_) {
    venc_module_1080p_ = std::make_shared<VencModule>();
  }

  if (running_venc_720p_) {
    venc_module_720p_ = std::make_shared<VencModule>();
  }
}

void SmartPlugin::ParseConfig() {
  xstream_workflow_cfg_file_ =
      config_->GetSTDStringValue("xstream_workflow_file");
  enable_profile_ = config_->GetBoolValue("enable_profile");
  profile_log_file_ = config_->GetSTDStringValue("profile_log_path");
  if (config_->HasMember("enable_result_to_json")) {
    result_to_json = config_->GetBoolValue("enable_result_to_json");
  }

  if (config_->HasMember("box_face_thr")) {
    smart_vo_cfg_.box_face_thr = config_->GetFloatValue("box_face_thr");
  }
  if (config_->HasMember("box_head_thr")) {
    smart_vo_cfg_.box_head_thr = config_->GetFloatValue("box_head_thr");
  }
  if (config_->HasMember("box_body_thr")) {
    smart_vo_cfg_.box_body_thr = config_->GetFloatValue("box_body_thr");
  }
  if (config_->HasMember("lmk_thr")) {
    smart_vo_cfg_.lmk_thr = config_->GetFloatValue("lmk_thr");
  }
  if (config_->HasMember("kps_thr")) {
    smart_vo_cfg_.kps_thr = config_->GetFloatValue("kps_thr");
  }
  if (config_->HasMember("box_veh_thr")) {
    smart_vo_cfg_.box_veh_thr = config_->GetFloatValue("box_veh_thr");
  }
  if (config_->HasMember("plot_fps")) {
    smart_vo_cfg_.plot_fps = config_->GetBoolValue("plot_fps");
  }

  rtsp_config_file_ = config_->GetSTDStringValue("rtsp_config_file");
  display_config_file_ = config_->GetSTDStringValue("display_config_file");

  LOGI << "xstream_workflow_file:" << xstream_workflow_cfg_file_;
  LOGI << "enable_profile:" << enable_profile_
       << ", profile_log_path:" << profile_log_file_;
}

int SmartPlugin::Init() {
  GetRtspConfigFromFile(rtsp_config_file_);
  LOGD << "get channel_num from file is:" << channel_num_;

  // init for xstream sdk
  LOGI << "smart plugin init";
  sdk_.resize(channel_num_);

  for (int i = 0; i < channel_num_; i++) {
    sdk_[i].reset(xstream::XStreamSDK::CreateSDK());
    sdk_[i]->SetConfig("config_file", xstream_workflow_cfg_file_);
    if (sdk_[i]->Init() != 0) {
      LOGE << "smart plugin init failed!!!";
      return kHorizonVisionInitFail;
    }

    sdk_[i]->SetCallback(
        std::bind(&SmartPlugin::OnCallback, this, std::placeholders::_1));
  }

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&SmartPlugin::Feed, this, std::placeholders::_1));

  if (running_vot_) {
    PipeModuleInfo module_info;
    vot_module_->SetDisplayMode(display_mode_);
    vot_module_->SetChannelNum(channel_num_);
    vot_module_->Init(0, &module_info, smart_vo_cfg_);
  }

  smart_venc_cfg_.input_num = channel_num_;

  if (running_venc_1080p_) {
    VencModuleInfo module_info_venc;
    module_info_venc.width = 1920;
    module_info_venc.height = 1080;
    module_info_venc.type = 1;
    module_info_venc.bits = 2000;

    venc_module_1080p_->Init(0, &module_info_venc, smart_venc_cfg_);
    venc_module_1080p_->Start();
  }

  if (running_venc_720p_) {
    VencModuleInfo module_info_venc;
    module_info_venc.width = 1280;
    module_info_venc.height = 720;
    module_info_venc.type = 1;
    module_info_venc.bits = 2000;

    venc_module_720p_->Init(1, &module_info_venc, smart_venc_cfg_);
    venc_module_720p_->Start();
  }

  return XPluginAsync::Init();
}

int SmartPlugin::Feed(XProtoMessagePtr msg) {
  // parse valid frame from msg
  LOGD << "smart plugin got one msg";
  auto valid_frame = std::static_pointer_cast<VioMessage>(msg);
  xstream::InputDataPtr input = Convertor::ConvertInput(valid_frame.get());
  SmartInput *input_wrapper = new SmartInput();
  if (input_wrapper == NULL) {
    LOGE << "new smart input ptr fail, return error!";
    return kHorizonVisionFailure;
  }
  input_wrapper->frame_info = valid_frame;
  input_wrapper->context = input_wrapper;

  int channel_id = input_wrapper->frame_info->channel_;
  monitor_->PushFrame(input_wrapper);
  monitor_->FrameStatistic(channel_id);

  if (channel_id >= channel_num_) {
    LOGE << "there is no channel num:" << channel_id << "for feed stream!!!";
    return -1;
  }

  input->context_ = (const void *)((uintptr_t)channel_id);
  if (sdk_[channel_id]->AsyncPredict(input) != 0) {
    return kHorizonVisionFailure;
  }

  LOGD << "Feed one task to xtream workflow, channel_id " << channel_id
       << " frame_id " << valid_frame->image_[0]->frame_id;

  return 0;
}

int SmartPlugin::Start() {
  LOGI << "SmartPlugin Start";
  root.clear();

  running_ = true;
  smartframe_ = 0;
  // read_thread_ = std::thread(&SmartPlugin::ComputeFpsThread, this);
  return 0;
}

int SmartPlugin::Stop() {
  running_ = false;
  // read_thread_.join();
  if (running_vot_) {
    vot_module_->Stop();
  }

  if (running_venc_1080p_) {
    venc_module_1080p_->Stop();
  }
  if (running_venc_720p_) {
    venc_module_720p_->Stop();
  }

  if (result_to_json) {
    remove("smart_data.json");
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream("smart_data.json");
    writer->write(root, &outputFileStream);
  }
  LOGI << "SmartPlugin Stop";
  return 0;
}

void SmartPlugin::OnCallback(xstream::OutputDataPtr xstream_out) {
  // On xstream async-predict returned,
  // transform xstream standard output to smart message.
  LOGI << "smart plugin got one smart result";
  HOBOT_CHECK(!xstream_out->datas_.empty()) << "Empty XStream Output";
  XStreamImageFramePtr *rgb_image = nullptr;

  for (const auto &output : xstream_out->datas_) {
    LOGD << output->name_ << ", type is " << output->type_;
    if (output->name_ == "rgb_image" || output->name_ == "image") {
      rgb_image = dynamic_cast<XStreamImageFramePtr *>(output.get());
    }
  }
  HOBOT_CHECK(rgb_image);

  smartframe_++;
  int channel_id = static_cast<int>((uintptr_t)xstream_out->context_);
  LOGD << "OnCallback channel id " << channel_id << " frame_id "
       << rgb_image->value->frame_id;

  if (running_vot_) {
    int layer =
        DisplayInfo::computePymLayer(display_mode_, channel_num_, channel_id);
    VotData vot_data;
    vot_data.y_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[layer]
            .y_vaddr);
    vot_data.uv_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[layer]
            .c_vaddr);

    vot_data.channel = channel_id;
    vot_data.width = ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
                         ->down_scale[layer]
                         .width;
    vot_data.height = ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
                          ->down_scale[layer]
                          .height;
    vot_module_->Input(&vot_data, xstream_out);
  }

  if (running_venc_1080p_) {
    VencData venc_data;
    venc_data.width = 960;
    venc_data.height = 540;
    venc_data.y_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[4]
            .y_vaddr);
    venc_data.uv_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[4]
            .c_vaddr);
    venc_data.channel = channel_id;
    venc_module_1080p_->Input(&venc_data, xstream_out);
  }

  if (running_venc_720p_) {
    VencData venc_data;
    venc_data.width = 640;
    venc_data.height = 360;
    venc_data.y_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[5]
            .y_vaddr);
    venc_data.uv_virtual_addr = reinterpret_cast<char *>(
        ((hobot::vision::PymImageFrame *)(rgb_image->value.get()))
            ->down_scale[5]
            .c_vaddr);
    venc_data.channel = channel_id;
    venc_module_720p_->Input(&venc_data, xstream_out);
  }

  auto input = monitor_->PopFrame(rgb_image->value->frame_id, channel_id);
  delete static_cast<SmartInput *>(input.context);
}

void SmartPlugin::GetRtspConfigFromFile(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOGE << "Open config file " << path << " failed";
    return;
  }
  Json::Value config_;
  ifs >> config_;
  ifs.close();

  auto value_js = config_["channel_num"];
  if (value_js.isNull()) {
    LOGE << "Can not find key: channel_num";
  }
  channel_num_ = value_js.asInt();

  value_js = config_["display_mode"];
  if (value_js.isNull()) {
    LOGE << "Can not find key: display_mode";
  }
}

void SmartPlugin::GetDisplayConfigFromFile(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOGE << "Open config file " << path << " failed";
    return;
  }
  Json::Value config_;
  ifs >> config_;
  ifs.close();

  running_vot_ = config_["vo"]["enable"].asBool();
  display_mode_ = config_["vo"]["display_mode"].asInt();

  running_venc_1080p_ = config_["rtsp"]["stream_1080p"].asBool();
  running_venc_720p_ = config_["rtsp"]["stream_720p"].asBool();
}

void SmartPlugin::ComputeFpsThread(void *param) {
  SmartPlugin *inst = reinterpret_cast<SmartPlugin *>(param);
  struct timeval start_time, finish_time;
  double timeuse = 0;
  double fps = 0;
  gettimeofday(&start_time, NULL);

  while (inst->running_) {
    sleep(10);
    gettimeofday(&finish_time, NULL);
    timeuse = finish_time.tv_sec - start_time.tv_sec +
              (finish_time.tv_usec - start_time.tv_usec) / 1000000.0;
    fps = inst->smartframe_ / timeuse;
    LOGD << "SmartPlugin use time:" << timeuse
         << ", smartframe:" << inst->smartframe_ << ", output fps =" << fps;
  }
}

}  // namespace smartplugin_multiplebox
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
