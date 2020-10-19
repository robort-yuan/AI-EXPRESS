/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:30:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_MULTIVIOPLUGIN_VIOCONFIG_H_
#define INCLUDE_MULTIVIOPLUGIN_VIOCONFIG_H_

#include <mutex>
#include <string>

#include "json/json.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

class VioConfig {
 public:
  static VioConfig *GetConfig();

  VioConfig() = default;

  explicit VioConfig(const std::string &path, const Json::Value &json)
      : path_(path), json_(json) {}

  std::string GetValue(const std::string &key) const;

  int GetIntValue(const std::string &key) const;

  Json::Value GetJson() const;

  bool SetConfig(VioConfig *config);

 private:
  static VioConfig *config_;
  std::string path_;
  Json::Value json_;
  mutable std::mutex mutex_;
};

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_MULTIVIOPLUGIN_VIOCONFIG_H_
