/*
 * Copyright (c) 2020-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     hid_manager.h
 * \Author   zhe.sun
 * \Version  1.0.0.0
 * \Date     2020/6/10
 * \Brief    implement of api header
 */
#ifndef INCLUDE_UVCPLUGIN_HIDMANAGER_H_
#define INCLUDE_UVCPLUGIN_HIDMANAGER_H_
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "./smart_manager.h"
#include "json/json.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/smartplugin_data.h"

using horizon::vision::xproto::basic_msgtype::SmartMessage;
using SmartMessagePtr = std::shared_ptr<SmartMessage>;

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XProtoMessagePtr;

class HidManager : public SmartManager {
 public:
  HidManager() = delete;
  explicit HidManager(std::string config_path);
  ~HidManager();
  virtual int Init();
  virtual int Start();
  virtual int Stop();

  virtual int FeedSmart(XProtoMessagePtr msg, int ori_image_width,
                        int ori_image_height, int dst_image_width,
                        int dst_image_height);

 private:
  int Send(const std::string &proto_str);
  void SendThread();
  int Serialize(SmartMessagePtr smart_msg, int ori_image_width,
                int ori_image_height, int dst_image_width,
                int dst_image_height);

 private:
  bool stop_flag_;
  std::string config_path_;
  Json::Value config_;
  std::shared_ptr<std::thread> thread_;

  enum SmartType { SMART_FACE, SMART_BODY, SMART_VEHICLE };
  SmartType smart_type_ = SMART_BODY;

  std::string hid_file_ = "/dev/hidg0";
  int hid_file_handle_ = -1;
  std::mutex queue_lock_;
  const unsigned int queue_max_size_ = 5;
  std::queue<std::string> pb_buffer_queue_;
  std::condition_variable condition_;

  hobot::CThreadPool serialize_thread_;
  bool print_timestamp_ = false;
};

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon

#endif  // INCLUDE_UVCPLUGIN_HIDMANAGER_H_
