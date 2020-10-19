/*
 * @Description: implement of can plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-10 19:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-09-10 19:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_CANPLUGIN_CANPLUGIN_H_
#define INCLUDE_CANPLUGIN_CANPLUGIN_H_

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <thread>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/can_bus_data.h"

#include "include/zmq_sub.h"
#include "include/zmq_pub.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace canplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::basic_msgtype::CanBusFromMcuMessage;
using horizon::vision::xproto::basic_msgtype::CanBusToMcuMessage;

class CanPlugin : public XPluginAsync {
 public:
  explicit CanPlugin(const std::string config_file);
  ~CanPlugin();
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  void CreateCanPlugin(const std::string pub_ipc, const std::string sub_ipc);
  void FeedCanBusFromMcuProc();
  int FeedCanBusToMcuMessage(XProtoMessagePtr msg);
  bool to_exit_ = false;
  std::string config_file_;
  std::thread *sub_thread_ = nullptr;   // receive from mcu, send to xproto bus
  Pub *can_output_ = nullptr;
  Sub *can_input_ = nullptr;
};

}  // namespace canplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_CANPLUGIN_CANPLUGIN_H_
