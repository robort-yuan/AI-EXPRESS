/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "multivioplugin/vioconfig.h"
#include <string>
#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

VioConfig *VioConfig::config_ = nullptr;

VioConfig *VioConfig::GetConfig() {
  if (config_ != nullptr) {
    return config_;
  } else {
    return nullptr;
  }
}

std::string VioConfig::GetValue(const std::string &key) const {
  std::lock_guard<std::mutex> lk(mutex_);
  if (json_[key].empty()) {
    LOGW << "Can not find key: " << key;
    return "";
  }

  return json_[key].asString();
}

int VioConfig::GetIntValue(const std::string &key) const {
  std::lock_guard<std::mutex> lk(mutex_);
  if (json_[key].empty()) {
    LOGF << "Can not find key: " << key;
    return -1;
  }
  return json_[key].asInt();
}

Json::Value VioConfig::GetJson() const { return this->json_; }

bool VioConfig::SetConfig(VioConfig *config) {
  if (config != nullptr) {
    config_ = config;
    return true;
  } else {
    return false;
  }
}

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
