/* @Description: declaration of websocketplg_helper
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-29 19:55:56
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-30 15:07:57
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#ifndef INCLUDE_WEBSOCKETPLG_HELPER_H_
#define INCLUDE_WEBSOCKETPLG_HELPER_H_

#include <string>
#include <memory>
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "websocketplugin/websocketplugin.h"
#include "pybind11/pybind11.h"

namespace horizon {
namespace vision {
namespace xproto {

namespace py = pybind11;

class WebsocketPlgHelper {
 public:
  explicit WebsocketPlgHelper(const std::string cfg_path);
  int Start();
  int Stop();
  void InvokeNativeFeedVideo(py::object msg);
  void InvokeNativeFeedSmart(py::object msg);

 private:
  std::shared_ptr<websocketplugin::WebsocketPlugin> websocket_plg_;
};

}   // namespace xproto
}   // namespace vision
}   // namespace horizon

#endif    // INCLUDE_WEBSOCKETPLG_HELPER_H_
