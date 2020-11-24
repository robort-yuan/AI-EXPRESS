/*!
 * Copyright (c) 2020-present, Horizon Robotics, Inc.
 * All rights reserved.
 * \File     hid_manager.cpp
 * \Author   zhe.sun
 * \Version  1.0.0.0
 * \Date     2020.6.10
 * \Brief    implement of api file
 */
#include "./hid_manager.h"

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
#include "smartplugin/smartplugin.h"
#ifdef USE_MC
#include "mcplugin/mcmessage.h"
#endif
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
#ifdef USE_MC
using horizon::vision::xproto::mcplugin::MCMessage;
#endif
HidManager::HidManager(std::string config_path) {
  config_path_ = config_path;
  LOGI << "HidManager smart config file path:" << config_path_;
  stop_flag_ = false;
  thread_ = nullptr;
}

HidManager::~HidManager() {}

int HidManager::Init() {
  LOGI << "HidManager Init";
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

    // hid_file_
    if (config_.isMember("hid_file")) {
      hid_file_ = config_["hid_file"].asString();
    }

    if (config_.isMember("ap_mode")) {
      ap_mode_ = config_["ap_mode"].asBool();
    }
  }

  // hid_file_handle_
  hid_file_handle_ = open(hid_file_.c_str(), O_RDWR, 0666);
  if (hid_file_handle_ < 0) {
    LOGE << "open hid device file fail: " << strerror(errno);
    return -1;
  }
  LOGD << "Hid open hid_file_handle";

  serialize_thread_.CreatThread(1);
  auto print_timestamp_str = getenv("uvc_print_timestamp");
  if (print_timestamp_str && !strcmp(print_timestamp_str, "ON")) {
    print_timestamp_ = true;
  }
  return 0;
}

int HidManager::DeInit() {
  close(hid_file_handle_);
  return 0;
}

int HidManager::Recv(char *recv_data) {
  int recv_size = 0;
  int ret = 0;
  bool is_readsize = false;
  int target_size = INT_MAX - sizeof(int);
  fd_set rset;        // 创建文件描述符的聚合变量
  timeval timeout;    // select timeout
  while (!stop_flag_) {
    if (recv_size == target_size + static_cast<int>(sizeof(int)))
      return recv_size;
    else if (recv_size > target_size + static_cast<int>(sizeof(int)))
      HOBOT_CHECK(false) << "recv_size is larger than target";
    FD_ZERO(&rset);     // 文件描述符聚合变量清0
    FD_SET(hid_file_handle_, &rset);  // 添加文件描述符
    timeout.tv_sec = 0;
    timeout.tv_usec = 100 * 1000;
    ret = select(hid_file_handle_ + 1, &rset, NULL, NULL, &timeout);
    if (ret == 0) {
      return recv_size;
    } else if (ret < 0) {
      LOGE << "Hid select: read request error, ret: " << ret;
      return ret;
    } else if (!FD_ISSET(hid_file_handle_, &rset)) {
      LOGE << "FD_ISSET is no";
      continue;
    }
    int remain = target_size + static_cast<int>(sizeof(int)) - recv_size;
    remain = remain >= HID_MAX_PACKET_SIZE ? HID_MAX_PACKET_SIZE : remain;
    ret = read(hid_file_handle_, recv_data + recv_size, remain);
    if (ret < 0) {
      LOGE << "Hid read hid_file_handle error, hid_file_handle_: "
           << hid_file_handle_ << " ret: " << ret;
      return ret;
    } else if (ret == 0) {
      continue;
    } else {
      recv_size += ret;
      if (!is_readsize && recv_size >= static_cast<int>(sizeof(int))) {
        is_readsize = true;
        target_size = *reinterpret_cast<int *>(recv_data);
        LOGI << "recv_size : " << recv_size;
        LOGI << "target_size: " << target_size;
      }
    }
  }
  return recv_size;
}
void HidManager::RecvThread() {
  char *recv_data = new char[HID_BUFFER_SIZE];
  while (!stop_flag_) {
    std::string recvstr;
    int recv_size = Recv(recv_data);
    if (recv_size > 0) {
      recvstr = std::string(recv_data + sizeof(int), recv_size - sizeof(int));
      LOGD << "Receive pb info data from ap size: " << recv_size;
      pb_ap2cp_info_queue_.push(recvstr);
    }
  }
  delete []recv_data;
}
void HidManager::SendThread() {
  // start send Hid 数据
  LOGD << "start HidManager";
  if (ap_mode_) {
//  char *recv_data = new char[1024];  // 接收host侧请求
    while (!stop_flag_) {
      // send smart
      {
        // 需要从pb_buffer中获取一个结果返回
        std::unique_lock<std::mutex> lck(queue_lock_);
        if (pb_buffer_queue_.size() > 0) {
          LOGD << "pb_buffer_queue_ size:" << pb_buffer_queue_.size();
          // send smart data
          LOGD << "Get smart data";
          std::string pb_string = pb_buffer_queue_.front();
          pb_buffer_queue_.pop();
          lck.unlock();
          // 将pb string 发送给ap
          if (Send(pb_string)) {
            LOGE << "Hid Send error!";
          } else {
            LOGD << "Hid Send end";
          }
        }
      }

      // send smart first, in order to avoid receiving smart data
      // after recved stop cmd response in ap
      // send info data
      while (pb_cp2ap_info_queue_.size() > 0) {
        std::string pb_string;
        if (pb_cp2ap_info_queue_.try_pop(&pb_string,
                                         std::chrono::microseconds(1000))) {
          // 将pb string 发送给ap
          if (Send(pb_string)) {
            LOGE << "Hid Send error!";
          } else {
            LOGD << "Hid Send end";
          }
        }
      }
    }
  } else {
    fd_set rset;                     // 创建文件描述符的聚合变量
    timeval timeout;                 // select timeout
    char *recv_data = new char[20];  // 接收host侧请求

    while (!stop_flag_) {
      FD_ZERO(&rset);                   // 文件描述符聚合变量清0
      FD_SET(hid_file_handle_, &rset);  // 添加文件描述符
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
      int retv = select(hid_file_handle_ + 1, &rset, NULL, NULL, &timeout);
      if (retv == 0) {
        LOGD << "Hid select: read request time out";
        continue;
      } else if (retv < 0) {
        LOGE << "Hid select: read request error, ret: " << retv;
        continue;
      }
      int ret = read(hid_file_handle_, recv_data, 20);
      if (ret > 0 && strncmp(recv_data, "GetSmartResult", 14) == 0) {
        LOGD << "Receive GetSmartResult";
        std::string pb_string = "";

        {
          std::unique_lock<std::mutex> lck(queue_lock_);
          bool wait_ret =
              condition_.wait_for(
                  lck, std::chrono::milliseconds(10),
                  [&]() { return pb_buffer_queue_.size() > 0; });
          // 从pb_buffer中获取结果
          if (wait_ret != 0) {
            // send smart data
            pb_string = pb_buffer_queue_.front();
            pb_buffer_queue_.pop();
          }
        }
        // 将pb string 发送给ap
        auto send_start_time = std::chrono::system_clock::now();
        if (Send(pb_string)) {
          LOGE << "Hid Send error!";
        } else {
          LOGD << "Hid Send end";
        }
        auto send_end_time = std::chrono::system_clock::now();
        if (print_timestamp_) {
          auto duration_time =
              std::chrono::duration_cast<std::chrono::milliseconds>(
                  send_end_time - send_start_time);
          LOGW << "hid send " << pb_string.length()
               << ",  use : " << duration_time.count() << " ms";
        }
      }
    }
    delete[] recv_data;
  }
}

int HidManager::Start() {
  if (hid_file_handle_ < 0) {
    LOGE << "hid_file_handle_ is not ready";
    return -1;
  }

  if (thread_ == nullptr) {
    thread_ = std::make_shared<std::thread>(&HidManager::SendThread, this);
  }
  if (ap_mode_ && recv_thread_ == nullptr) {
    recv_thread_ = std::make_shared<std::thread>(
        &HidManager::RecvThread, this);
  }
  return 0;
}

int HidManager::Stop() {
  LOGI << "HidManager Stop";
  stop_flag_ = true;
  if (thread_ != nullptr) {
    thread_->join();
    thread_ = nullptr;
  }
  if (recv_thread_ != nullptr) {
    recv_thread_->join();
    recv_thread_ = nullptr;
  }
  LOGI << "HidManager Stop Done";
  return 0;
}

int HidManager::FeedInfo(const XProtoMessagePtr& msg) {
#ifdef USE_MC
  auto mc_msg = std::static_pointer_cast<MCMessage>(msg);
  if (!mc_msg.get()) {
    LOGE << "msg is null";
    return -1;
  }
  std::string protocol = mc_msg->Serialize();
  // pb入队
  LOGD << "info data to queue, size: " << protocol.size();
  pb_cp2ap_info_queue_.push(std::move(protocol));
#endif
  return 0;
}

int HidManager::FeedSmart(XProtoMessagePtr msg, int ori_image_width,
                          int ori_image_height, int dst_image_width,
                          int dst_image_height) {
  auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
  // convert pb2string
  if (!smart_msg.get()) {
    LOGE << "msg is null";
    return -1;
  }

  if (serialize_thread_.GetTaskNum() > 5) {
    LOGW << "Hid Serialize Thread task num more than 5: "
         << serialize_thread_.GetTaskNum();
  }
  serialize_thread_.PostTask(
      std::bind(&HidManager::Serialize, this, smart_msg,
                ori_image_width, ori_image_height,
                dst_image_width, dst_image_height));
  return 0;
}

int HidManager::Serialize(SmartMessagePtr smart_msg,
                          int ori_image_width, int ori_image_height,
                          int dst_image_width, int dst_image_height) {
  std::string protocol;
  uint64_t timestamp = 0;
  switch ((SmartType)smart_type_) {
    case SmartType::SMART_FACE:
    case SmartType::SMART_BODY: {
      auto msg = dynamic_cast<CustomSmartMessage *>(smart_msg.get());
      if (msg) {
        msg->SetAPMode(ap_mode_);
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
      LOGD << "Drop smartMsg data...";
    }
  }
  LOGD << "pb_buffer_queue_ size:" << pb_buffer_queue_.size();
  condition_.notify_one();
  if (print_timestamp_) {
    LOGW << "HidManager::Serialize timestamp:" << timestamp;
  }
  return 0;
}

int HidManager::Send(const std::string &proto_str) {
  LOGD << "Start send smart data...  len:" << proto_str.length();
  int buffer_size_src = proto_str.length();
  char *str_src = const_cast<char *>(proto_str.c_str());

  int buffer_size = buffer_size_src + sizeof(int);  // size
  char *buffer = new char[buffer_size];
  if (buffer == nullptr) {
    LOGE << "send error: null data!";
    return -1;
  }
  // add size
  memmove(buffer, &buffer_size, sizeof(int));
  memmove(buffer + sizeof(int), str_src, buffer_size_src);
  int ret = 0;
  char *buffer_offset = buffer;
  int remainding_size = buffer_size;

  fd_set wset;      // 创建文件描述符的聚合变量
  timeval timeout;  // select timeout
  while (remainding_size > 0 && !stop_flag_) {
    FD_ZERO(&wset);                   // 文件描述符聚合变量清0
    FD_SET(hid_file_handle_, &wset);  // 添加文件描述符
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int retv = select(hid_file_handle_ + 1, NULL, &wset, NULL, &timeout);
    if (retv == 0) {
      LOGD << "Hid select: send data time out";
      continue;
    } else if (retv < 0) {
      LOGE << "Hid select: send data error, ret: " << retv;
      continue;
    }
    if (remainding_size >= 1024) {
      LOGD << "Send 1024 bytes data...";
      ret = write(hid_file_handle_, buffer_offset, 1024);
      LOGD << "Send 1024 bytes data end";
    } else {
      LOGD << "Send " << remainding_size << " bytes data...";
      ret = write(hid_file_handle_, buffer_offset, remainding_size);
      LOGD << "Send " << remainding_size << " bytes data end";
    }
    if (ret < 0) {
      LOGF << "send package error: " << strerror(errno) << "; ret: " << ret;
      delete[] buffer;
      return -1;
    }
    remainding_size = remainding_size - ret;
    buffer_offset = buffer_offset + ret;
    if (remainding_size < 0) {
      LOGF << "send package error: " << strerror(errno) << "; ret: " << ret;
      delete[] buffer;
      return -1;
    }
  }
  delete[] buffer;
  return 0;
}

}  // namespace Uvcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
