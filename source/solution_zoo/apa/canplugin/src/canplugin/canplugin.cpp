/*
 * @Description: implement of can plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-10 19:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-09-10 19:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#include "canplugin/canplugin.h"

#include <string>
#include <thread>
#include <iostream>
#include <fstream>

#include "json/json.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "hobotlog/hobotlog.hpp"
#include "canplugin/can_data_type.h"

using horizon::vision::xproto::basic_msgtype::CanBusToMcuMessage;
using horizon::vision::xproto::basic_msgtype::CanBusToMcuMessagePtr;
using horizon::vision::xproto::basic_msgtype::CanBusFromMcuMessage;
using horizon::vision::xproto::basic_msgtype::CanBusFromMcuMessagePtr;

namespace horizon {
namespace vision {
namespace xproto {
namespace canplugin {

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_CAN_BUS_FROM_MCU_MESSAGE)
XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_CAN_BUS_TO_MCU_MESSAGE)


CanPlugin::CanPlugin(const std::string config_file) {
  config_file_ = config_file;
}

CanPlugin::~CanPlugin() {
  // todo, need to delete some data
}

int CanPlugin::Init() {
  std::string pub_ipc = "/tmp/can_output_extra.ipc";
  std::string sub_ipc = "/tmp/can_input_extra.ipc";
  std::ifstream ifs(config_file_);
  if (!ifs.is_open()) {
    LOGW << "open config file " << config_file_ << " failed, "
         << "use default config";
    return 0;
  }
  Json::CharReaderBuilder builder;
  std::string err_json;
  Json::Value json_obj;
  try {
    bool ret = Json::parseFromStream(builder, ifs, &json_obj, &err_json);
    if (!ret) {
      LOGF << "invalid config file " << config_file_;
      return -1;
    }
  } catch (std::exception &e) {
    LOGF << "exception while parse config file " << config_file_ << ", "
         << e.what();
    return -1;
  }
  if (json_obj.isMember("pub_ipc")) {
    pub_ipc = json_obj["pub_ipc"].asString();
  }
  if (json_obj.isMember("sub_ipc")) {
    sub_ipc = json_obj["sub_ipc"].asString();
  }
  CreateCanPlugin(pub_ipc, sub_ipc);

  RegisterMsg(TYPE_CAN_BUS_TO_MCU_MESSAGE,
              std::bind(&CanPlugin::FeedCanBusToMcuMessage, this,
                        std::placeholders::_1));
  sub_thread_ =
      new std::thread(std::bind(&CanPlugin::FeedCanBusFromMcuProc, this));
  if (nullptr == sub_thread_) {
    LOGF << "create thread failed";
    return -1;
  }
  sub_thread_->detach();
  LOGI << "can plugin init success";
  return XPluginAsync::Init();
}

int CanPlugin::Start() {
  to_exit_ = false;
  return 0;
}

int CanPlugin::Stop() {
  to_exit_ = true;
  return 0;
}

void CanPlugin::CreateCanPlugin(const std::string pub_ipc,
                                const std::string sub_ipc) {
  if (pub_ipc.size() > 0) {
    can_output_ = new Pub(pub_ipc);
    if (can_output_ == nullptr) {
      LOGF << "create can Pub failed";
      return;
    }
    int ret = can_output_->InitPub();
    if (ret != 0) {
      LOGF << "init can output failed";
      return;
    }
  }

  if (sub_ipc.size() > 0) {
    can_input_ = new Sub(sub_ipc);
    int ret = can_input_->InitSub();
    if (ret != 0) {
      LOGF << "init can input failed";
      return;
    }
  }
}

void CanPlugin::FeedCanBusFromMcuProc() {
  while (!to_exit_) {
    if (can_input_ == nullptr) {
      break;
    }
    CanBusFromMcuMessagePtr msg(new CanBusFromMcuMessage());
    int ret = can_input_->IpcSub(&(msg->can_data_[0]));
    if (ret > 0) {
      msg->can_data_len_ = ret;
      // example, parse the msg
      struct can_header * header = nullptr;
      struct can_frame * frame = nullptr;
      header = reinterpret_cast<struct can_header *>(&(msg->can_data_[0]));
      LOGI << "ts: " << header->time_stamp;
      LOGI << "rc: " << int(header->counter);
      LOGI << "number: " << int(header->frame_num);
      frame = reinterpret_cast<struct can_frame *>(msg->can_data_ +
                                                   sizeof(struct can_header));
      for (int i = 0; i < header->frame_num; ++i) {
        LOGI << "can_id: " << frame[i].can_id
             << " dlc: " << int(frame[i].can_dlc) << " data:";
        for (int j = 0; j < 8; j++) {
          LOGI << " " << int(frame[i].data[j]);
        }
      }
      // example end
      PushMsg(msg);
    }
  }
}

int CanPlugin::FeedCanBusToMcuMessage(XProtoMessagePtr msg) {
  LOGI << "got one message: can to mcu";
  auto to_mcu_msg = std::static_pointer_cast<CanBusToMcuMessage>(msg);
  if (can_output_ && !to_exit_) {
    int ret =
        can_output_->IpcPub(to_mcu_msg->can_data_, to_mcu_msg->can_data_len_);
    if (ret < 0) {
      LOGE << "IpcPub failed";
    } else {
      LOGI << "ipc pub success, send len: " << to_mcu_msg->can_data_len_;
    }
  }
  return 0;
}

}  // namespace canplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
