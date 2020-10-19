/*
 * @Description: implement of multi smart plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-26 09:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-29 22:45:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MULTISMARTPLUGIN_SMARTPLUGIN_H_
#define INCLUDE_MULTISMARTPLUGIN_SMARTPLUGIN_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "multismartplugin/runtime_monitor.h"
#include "multismartplugin/smart_config.h"
#include "../../multivioplugin/include/multivioplugin/viomessage.h"

#include "xproto_msgtype/smartplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace multismartplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::basic_msgtype::SmartMessage;

struct CustomSmartMessage : SmartMessage {
  explicit CustomSmartMessage(xstream::OutputDataPtr out)
      : smart_result_(out) {
    type_ = TYPE_SMART_MESSAGE;
  }
  std::string Serialize() override;
  std::string Serialize(int ori_w, int ori_h, int dst_w, int dst_h) override;
  void *ConvertData() override;
 private:
  xstream::OutputDataPtr smart_result_;
};

class SmartPlugin : public XPluginAsync {
 public:
  SmartPlugin() = default;
  explicit SmartPlugin(const std::string &config_file);

  void SetConfig(const std::string &config_file) { config_file_ = config_file; }

  ~SmartPlugin() = default;
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  int Feed(XProtoMessagePtr msg);
  int FeedIpm(XProtoMessagePtr msg);
  int FeedMulti(XProtoMessagePtr msg);
  void OnCallback(xstream::OutputDataPtr out);
  void ParseConfig();
  int OverWriteSourceNum(const std::string &cfg_file, int source_num = 1);

  std::vector<std::shared_ptr<XStreamSDK>> sdk_;
  std::shared_ptr<RuntimeMonitor> monitor_;
  std::string config_file_;
  // source id => [target workflow, ...]
  std::map<int, std::vector<int>> source_target_;
  // xstream instance => input source list
  std::vector<std::vector<int>> source_map_;

  std::shared_ptr<JsonConfigWrapper> config_;
};

}  // namespace multismartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_MULTISMARTPLUGIN_SMARTPLUGIN_H_
