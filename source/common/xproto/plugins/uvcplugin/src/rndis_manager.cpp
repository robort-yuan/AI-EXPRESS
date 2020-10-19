/*!
 * Copyright (c) 2020-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     rndis_manager.cpp
 * \Author   xudong.du
 * \Version  1.0.0.0
 * \Date     2020.9.27
 * \Brief    implement of api file
 */
#include "./rndis_manager.h"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <string>

#include "hobotlog/hobotlog.hpp"
#include "hobotxsdk/xstream_data.h"
#include "smartplugin/smartplugin.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace Uvcplugin {
using horizon::vision::xproto::XPluginErrorCode;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::smartplugin::CustomSmartMessage;
using horizon::vision::xproto::smartplugin::VehicleSmartMessage;

RndisManager::RndisManager(std::string config_path) {
  config_path_ = config_path;
  LOGI << "RndisManager smart config file path:" << config_path_;
  stop_flag_ = false;
  thread_ = nullptr;
  server_context_ = std::make_shared<zmq::context_t>(1);
  smart_publisher_ = std::make_shared<zmq::socket_t>(*server_context_, ZMQ_PUB);
}

RndisManager::~RndisManager() {}

int RndisManager::Init() {
  LOGI << "RndisManager Init";
  // config_
  if (config_path_ != "") {
    std::ifstream ifs(config_path_);
    if (!ifs.is_open()) {
      LOGF << "open config file " << config_path_ << " failed";
      return -1;
    }
    Json::CharReaderBuilder builder;
    std::string err_json;
    try {
      bool ret = Json::parseFromStream(builder, ifs, &config_, &err_json);
      if (!ret) {
        LOGF << "invalid config file " << config_path_;
        return -1;
      }
    } catch (std::exception &e) {
      LOGF << "exception while parse config file " << config_path_ << ", "
           << e.what();
      return -1;
    }
    // smart_type_
    if (config_.isMember("smart_type")) {
      smart_type_ = static_cast<SmartType>(config_["smart_type"].asInt());
    }

    // rndis_port
    if (config_.isMember("rndis_port")) {
      smart_port_ = config_["rndis_port"].asInt();
    }
  }

  // start zmq server
  std::string tcpaddr = "tcp://0.0.0.0:";
  tcpaddr += std::to_string(smart_port_);
  try {
    smart_publisher_->bind(tcpaddr);
    server_ok_ = true;
  } catch (std::exception e) {
    LOGE << "bind port: " << smart_port_ << " failed, " << e.what();
    server_ok_ = false;
  }

  serialize_thread_.CreatThread(1);
  auto print_timestamp_str = getenv("uvc_print_timestamp");
  if (print_timestamp_str && !strcmp(print_timestamp_str, "ON")) {
    print_timestamp_ = true;
  }
  LOGW << "rndis init ok";
  return 0;
}

void RndisManager::SendThread() {
  // start send smart 数据
  LOGD << "start RndisManager";
  std::string pb_string;
  while (!stop_flag_) {
    {
      std::unique_lock<std::mutex> lck(queue_lock_);
      bool wait_ret =
          condition_.wait_for(lck, std::chrono::milliseconds(10),
                              [&]() { return pb_buffer_queue_.size() > 0; });
      // 从pb_buffer中获取结果
      if (wait_ret != 0) {
        // send smart data
        pb_string = pb_buffer_queue_.front();
        pb_buffer_queue_.pop();
      }
    }
    if (false == server_ok_ || pb_string.empty()) {
      continue;
    }
    // 将pb string 发送给ap
    auto send_start_time = std::chrono::system_clock::now();
    int buffer_size = pb_string.length();
    char *buffer = const_cast<char *>(pb_string.c_str());
    zmq::message_t message(buffer_size);
    memmove(message.data(), buffer, buffer_size);
    smart_publisher_->send(message, zmq::send_flags::none);
    auto send_end_time = std::chrono::system_clock::now();

    if (print_timestamp_) {
      auto duration_time =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              send_end_time - send_start_time);
      LOGD << "rndis send " << buffer_size
           << ",  use : " << duration_time.count() << " ms";
    }
  }
}

int RndisManager::Start() {
  if (thread_ == nullptr) {
    thread_ = std::make_shared<std::thread>(&RndisManager::SendThread, this);
  }
  return 0;
}

int RndisManager::Stop() {
  LOGI << "RndisManager Stop";
  stop_flag_ = true;
  if (thread_ != nullptr) {
    thread_->join();
  }
  return 0;
}

int RndisManager::FeedSmart(XProtoMessagePtr msg, int ori_image_width,
                            int ori_image_height, int dst_image_width,
                            int dst_image_height) {
  auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
  // convert pb2string
  if (!smart_msg.get()) {
    LOGE << "msg is null";
    return -1;
  }

  if (serialize_thread_.GetTaskNum() > 5) {
    LOGW << "Serialize Thread task num more than 5: "
         << serialize_thread_.GetTaskNum();
  }
  serialize_thread_.PostTask(
      std::bind(&RndisManager::Serialize, this, smart_msg, ori_image_width,
                ori_image_height, dst_image_width, dst_image_height));
  return 0;
}

int RndisManager::Serialize(SmartMessagePtr smart_msg, int ori_image_width,
                            int ori_image_height, int dst_image_width,
                            int dst_image_height) {
  std::string protocol;
  uint64_t timestamp = 0;
  switch ((SmartType)smart_type_) {
    case SmartType::SMART_FACE:
    case SmartType::SMART_BODY: {
      auto msg = dynamic_cast<CustomSmartMessage *>(smart_msg.get());
      if (msg) {
        protocol = msg->Serialize(ori_image_width, ori_image_height,
                                  dst_image_width, dst_image_height);
        timestamp = msg->time_stamp;
      }
      break;
    }
    case SmartType::SMART_VEHICLE: {
      auto msg = dynamic_cast<VehicleSmartMessage *>(smart_msg.get());
      if (msg) {
        protocol = msg->Serialize(ori_image_width, ori_image_height,
                                  dst_image_width, dst_image_height);
        timestamp = msg->time_stamp;
      }
      break;
    }
    default:
      LOGE << "not support smart_type";
      return -1;
  }

  // pb入队
  LOGD << "smart data to queue";
  {
    std::lock_guard<std::mutex> lck(queue_lock_);
    pb_buffer_queue_.push(protocol);  // 将新的智能结果放入队列尾部
    if (pb_buffer_queue_.size() > queue_max_size_) {
      pb_buffer_queue_.pop();  // 将队列头部的丢弃
      LOGW << "Drop smartMsg data...";
    }
  }
  condition_.notify_one();
  if (print_timestamp_) {
    LOGW << "RndisManager::Serialize timestamp:" << timestamp;
  }
  return 0;
}

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
