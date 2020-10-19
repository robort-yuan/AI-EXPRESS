/* @Description: implementation of websocketplg_helper
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-30 12:02:02
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-30 15:09:37
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#include "websocketplg_helper.h"

namespace horizon {
namespace vision {
namespace xproto {

namespace py = pybind11;

WebsocketPlgHelper::WebsocketPlgHelper(const std::string cfg_path) {
  websocket_plg_ = std::make_shared<websocketplugin::WebsocketPlugin>(cfg_path);
}

int WebsocketPlgHelper::Start() {
  HOBOT_CHECK(websocket_plg_);
  int ret = websocket_plg_->Init();
  if (ret != 0) {
    std::cout << "Failed to init native websocket plugin, code: "
              << ret << std::endl;
    return ret;
  }
  ret = websocket_plg_->Start();
  if (ret != 0) {
    std::cout << "Failed to start native websocket plugin, code: "
              << ret << std::endl;
    return ret;
  }
  return 0;
}

int WebsocketPlgHelper::Stop() {
  HOBOT_CHECK(websocket_plg_);
  int ret = websocket_plg_->Stop();
  if (ret != 0) {
    std::cout << "Failed to stop native websocket plugin, code: "
              << ret << std::endl;
    return ret;
  }
  ret = websocket_plg_->DeInit();
  if (ret != 0) {
    std::cout << "Failed to deinit native websocket plugin, code: "
              << ret << std::endl;
    return ret;
  }
  return 0;
}

void WebsocketPlgHelper::InvokeNativeFeedVideo(py::object message) {
  XProtoMessagePtr flow_msg = message.cast<XProtoMessagePtr>();
  auto vio_msg = std::static_pointer_cast<basic_msgtype::VioMessage>(flow_msg);
  std::cout << "C++ WebPlgHelper convert py::object to vio_msg" << std::endl;
  websocket_plg_->FeedVideo(vio_msg);
  std::cout << "C++ returned from feedvideo" << std::endl;
}

void WebsocketPlgHelper::InvokeNativeFeedSmart(py::object message) {
  XProtoMessagePtr flow_msg = message.cast<XProtoMessagePtr>();
  auto smart_msg =
      std::static_pointer_cast<basic_msgtype::SmartMessage>(flow_msg);
  std::cout << "C++ WebPlgHelper convert py::object to smart_msg" << std::endl;
  websocket_plg_->FeedSmart(smart_msg);
  std::cout << "C++ returned from feedsmart" << std::endl;
}

}   // namespace xproto
}   // namespace vision
}   // namespace horizon
