/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-22 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "multivioplugin/vioplugin.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/msg_registry.h"

#include "multivioplugin/vioproduce.h"

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_IMAGE_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_DROP_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_MULTI_IMAGE_MESSAGE)
namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

VioConfig *VioPlugin::GetConfigFromFile(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOGF << "Open config file " << path << " failed";
    return nullptr;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  ifs.close();
  std::string content = ss.str();
  Json::Value value;
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  JSONCPP_STRING error;
  std::shared_ptr<Json::CharReader> reader(builder.newCharReader());
  bool ret = reader->parse(content.c_str(), content.c_str() + content.size(),
                             &value, &error);
  if (ret) {
    auto *config = new VioConfig(path, value);
    return config;
  } else {
    LOGF << "parse multivioplugin failed:" << path;
    return nullptr;
  }
}

VioPlugin::VioPlugin(const std::string &path) {
  config_ = GetConfigFromFile(path);
  config_->SetConfig(config_);
  max_buffer_size_ = 2;
  is_msg_package_ = 0;
  HOBOT_CHECK(config_);
}

int VioPlugin::Init() {
  auto data_source = config_->GetValue("data_source");
  int data_source_num = config_->GetIntValue("data_source_num");
  is_msg_package_ = config_->GetIntValue("is_msg_package");
  chn_grp_.push_back(0);
  chn_grp_.push_back(1);
  chn_grp_.push_back(2);
  chn_grp_.push_back(3);
  HOBOT_CHECK(data_source_num > 0) << "data_source_num config error";
  LOGI << "data source num:" << data_source_num;
  vio_produce_handle_.resize(data_source_num);
  {
    std::lock_guard<std::mutex> lk(pym_img_mutex_);
    pym_img_buffer_.resize(data_source_num);
    for (int i = 0; i < data_source_num; ++i) {
      channel_status_[i] = 1;
      pym_img_buffer_[i] =
          new hobot::vision::BlockingQueue<std::shared_ptr<VioMessage>>();
    }
  }
  LOGI << "create pym_img_buffer_ success";

  auto send_frame = [&](const std::shared_ptr<VioMessage> input) {
    if (!input) {
      LOGE << "VioMessage is NULL, return";
      return -1;
    }
    // push message to xproto
    // PushMsg(input);
    int channel_id = input->channel_;
    int channel_size = pym_img_buffer_.size();
    if (channel_id < 0 || channel_id >= channel_size) {
      LOGF << "vio produce do not set channel id";
    }
    {
      std::lock_guard<std::mutex> lk(pym_img_mutex_);
      while (pym_img_buffer_[channel_id]->size() >= max_buffer_size_) {
        pym_img_buffer_[channel_id]->pop();
      }
      LOGI << "chn " << channel_id << " push viomessage";
      pym_img_buffer_[channel_id]->push(input);
    }
    if (!is_msg_package_) {
      SyncPymImage();
    } else {
      SyncPymImages();
    }
    return 0;
  };

  LOGI << "begin create vio produce";
  for (int i = 0; i < data_source_num; ++i) {
    LOGI << "begin call CreateVioProduce";
    vio_produce_handle_[i] = VioProduce::CreateVioProduce(data_source);
    LOGI << "after call CreateVioProduce";
    HOBOT_CHECK(vio_produce_handle_[i]) << "create vio produce failed";
    LOGI << "create vio produce success";
    vio_produce_handle_[i]->SetConfig(config_, i);
    vio_produce_handle_[i]->SetListener(send_frame);
  }
  // 调用父类初始化成员函数注册信息
  XPluginAsync::Init();
  is_inited_ = true;
  return 0;
}

VioPlugin::~VioPlugin() {
  if (config_) {
    delete config_;
  }
}

void VioPlugin::SyncPymImage() {
  std::lock_guard<std::mutex> lk(pym_img_mutex_);
  bool reset = true;
  for (auto itr = channel_status_.begin(); itr != channel_status_.end();
       ++itr) {
    int channel_id = itr->first;
    int send_status = itr->second;
    if (send_status) {
      // can send
      reset = false;
      if (pym_img_buffer_[channel_id]->size() > 0) {
        auto vio_msg = pym_img_buffer_[channel_id]->pop();
        PushMsg(vio_msg);
        itr->second = 0;
      }
    }
  }
  if (reset) {
    for (auto itr = channel_status_.begin(); itr != channel_status_.end();
         ++itr) {
      itr->second = 1;  // now can send
      int channel_id = itr->first;
      if (pym_img_buffer_[channel_id]->size() > 0) {
        auto vio_msg = pym_img_buffer_[channel_id]->pop();
        PushMsg(vio_msg);
        LOGI << "send vio message, ch:" << channel_id
             << ", frame_id:" << vio_msg->sequence_id_
             << ", timestamp:" << vio_msg->time_stamp_;
        itr->second = 0;
      }
    }
  }
  for (auto itr = channel_status_.begin(); itr != channel_status_.end();
         ++itr) {
    int channel_id = itr->first;
    int channel_status = itr->second;
    LOGD << "channel_id:" << channel_id << ", status:" << channel_status;
  }
}

void VioPlugin::SyncPymImages() {
  std::lock_guard<std::mutex> lk(pym_img_mutex_);

  for (auto itr = channel_status_.begin(); itr != channel_status_.end();
         ++itr) {
    int channel_id = itr->first;
    if (find(chn_grp_.begin(), chn_grp_.end(), channel_id)
       == chn_grp_.end()) {
      continue;
    } else if (pym_img_buffer_[channel_id]->size()) {
      continue;
    } else {
      return;
    }
  }

  std::shared_ptr<MultiVioMessage> multi_vio_msg(new MultiVioMessage());
  for (auto itr = channel_status_.begin(); itr != channel_status_.end();
         ++itr) {
    int channel_id = itr->first;
    if (find(chn_grp_.begin(), chn_grp_.end(), channel_id)
       != chn_grp_.end()) {
          auto vio_msg = std::static_pointer_cast<ImageVioMessage>(
            pym_img_buffer_[channel_id]->pop());
          multi_vio_msg->multi_vio_img_.push_back(vio_msg);
       }
  }

  PushMsg(multi_vio_msg);
}

int VioPlugin::Start() {
  int ret;
  for (auto &produce_handle : vio_produce_handle_) {
    ret = produce_handle->Start();
    if (ret < 0) {
      LOGF << "vio produce start failed, err: " << ret;
      return -1;
    }
  }
  return 0;
}

int VioPlugin::Stop() {
  for (auto &produce_handle : vio_produce_handle_) {
    produce_handle->Stop();
  }
  return 0;
}
}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
