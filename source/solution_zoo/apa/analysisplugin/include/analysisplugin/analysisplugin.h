/*
 * @Description: implement of analysis plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-14 15:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-09-14 15:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_ANALYSISPLUGIN_ANALYSISPLUGIN_H_
#define INCLUDE_ANALYSISPLUGIN_ANALYSISPLUGIN_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

#include "../../multivioplugin/include/multivioplugin/viomessage.h"
#include "xproto_msgtype/can_bus_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace analysisplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::multivioplugin::ImageVioMessage;
using horizon::vision::xproto::basic_msgtype::CanBusFromMcuMessage;


class AnalysisPlugin : public XPluginAsync {
 public:
  explicit AnalysisPlugin(const std::string &config_file);

  ~AnalysisPlugin() = default;
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  int FeedMultiVio(XProtoMessagePtr msg);
  int FeedCanFromMcu(XProtoMessagePtr msg);
  std::string config_file_;
};

}  // namespace analysisplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_ANALYSISPLUGIN_ANALYSISPLUGIN_H_
