/* 
 * @Description: gdcplugin declaration
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-04 20:29:27
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-08 20:10:29
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_GDCPLUGIN_GDCPLUGIN_H_
#define INCLUDE_GDCPLUGIN_GDCPLUGIN_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

#include "xproto_msgtype/smartplugin_data.h"
#include "multismartplugin/smart_config.h"
#include "gdcplugin/ipm_util.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace gdcplugin {

using hobot::vision::PymImageFrame;
using hobot::vision::CVImageFrame;
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::multismartplugin::JsonConfigWrapper;

class GdcPlugin : public XPluginAsync {
 public:
  GdcPlugin() = default;
  ~GdcPlugin() = default;
  explicit GdcPlugin(const std::string &config_file);
  void SetConfig(const std::string &config_file) { config_file_ = config_file; }
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  int OnVioMessage(XProtoMessagePtr msg);

 private:
  std::string config_file_;
  std::shared_ptr<JsonConfigWrapper> config_;
  bool all_in_one_vio_ = false;
  uint32_t data_source_num_;
  std::vector<std::string> config_files_;
  std::vector<int> chn2direction_;
  std::vector<int> concate_chns_;
  std::unordered_map<uint64_t, std::vector<std::shared_ptr<PymImageFrame>>>
    image_buffer_;
  IpmUtil ipm_util_;
};

}  // namespace gdcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_GDCPLUGIN_GDCPLUGIN_H_
