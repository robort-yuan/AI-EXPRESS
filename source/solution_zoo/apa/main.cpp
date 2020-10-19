/*
 * @Description: implement of apa main
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-31 09:30:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-31 20:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include <signal.h>
#include <unistd.h>
#include <malloc.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "json/json.h"

#include "hobotlog/hobotlog.hpp"
#include "multismartplugin/smartplugin.h"
#include "multivioplugin/vioplugin.h"
#include "multiwebsocketplugin/websocketplugin.h"
#include "gdcplugin/gdcplugin.h"
#include "displayplugin/displayplugin.h"
#include "canplugin/canplugin.h"
#include "canplugin/can_data_type.h"
#include "analysisplugin/analysisplugin.h"
#include "bpu_predict/bpu_predict_extension.h"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using std::chrono::seconds;

using horizon::vision::xproto::multismartplugin::SmartPlugin;
using horizon::vision::xproto::multivioplugin::VioPlugin;
using horizon::vision::xproto::gdcplugin::GdcPlugin;
using horizon::vision::xproto::displayplugin::DisplayPlugin;
using horizon::vision::xproto::canplugin::CanPlugin;
using horizon::vision::xproto::basic_msgtype::CanBusToMcuMessage;
using horizon::vision::xproto::basic_msgtype::CanBusToMcuMessagePtr;
using horizon::vision::xproto::canplugin::can_header;
using horizon::vision::xproto::canplugin::can_frame;
using horizon::vision::xproto::analysisplugin::AnalysisPlugin;

static bool exit_ = false;

// 用于生产can消息，向mcu发送can消息
class CanExamplePlugin : public XPluginAsync {
 public:
  CanExamplePlugin() {}
  ~CanExamplePlugin() {}
  int Init() {
    work_thread_ =
        new std::thread(std::bind(&CanExamplePlugin::WorkProc, this));
    work_thread_->detach();
    XPluginAsync::Init();
    return 0;
  }
  int Start() {
    return 0;
  }
  int Stop() {
    return 0;
  }
  std::string desc() const { return "CanExamplePlugin"; }

 private:
  void *WorkProc() {
    uint8_t pub_counter = 0;
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      CanBusToMcuMessagePtr msg(new CanBusToMcuMessage());
      struct can_header *header =
          reinterpret_cast<struct can_header *>(&(msg->can_data_[0]));
      header->time_stamp = 0;  // current ts
      header->counter = pub_counter;
      header->frame_num = 4;
      struct can_frame *frame = reinterpret_cast<struct can_frame *>(
          msg->can_data_ + sizeof(struct can_header));
      frame[0].can_id = 0x7F;
      frame[0].can_dlc = 8;
      frame[0].data[0] = pub_counter;
      frame[1].can_id = 0xC6;
      frame[1].can_dlc = 8;
      frame[1].data[0] = pub_counter;
      frame[2].can_id = 0x336;
      frame[2].can_dlc = 8;
      frame[2].data[0] = pub_counter;
      frame[3].can_id = 0x15E;
      frame[3].can_dlc = 8;
      frame[3].data[0] = pub_counter;
      msg->can_data_len_ = sizeof(struct can_header) + 16 * header->frame_num;
      PushMsg(msg);
      ++pub_counter;
    }
  }

 private:
  std::thread *work_thread_;
};

int main(int argc, char **argv) {
  mallopt(M_TRIM_THRESHOLD, 128 * 1024);
  HB_BPU_setGlobalConfig(BPU_GLOBAL_ENGINE_TYPE, "native");
  std::string run_mode = "ut";

  if (argc < 5) {
    std::cout << "Usage: smart_main vio_config_file "
              << "xstream_config_file visualplugin_config "
              << "[-i/-d/-w/-f] " << std::endl;
    return 0;
  }
  std::string vio_config_file = std::string(argv[1]);
  std::string smart_config_file = std::string(argv[2]);
  std::string websocket_config_file = std::string(argv[3]);
  std::string gdc_config_file = std::string(argv[4]);
  std::string display_config_file = std::string(argv[5]);

  std::string log_level(argv[6]);
  if (log_level == "-i") {
    SetLogLevel(HOBOT_LOG_INFO);
  } else if (log_level == "-d") {
    SetLogLevel(HOBOT_LOG_DEBUG);
  } else if (log_level == "-w") {
    SetLogLevel(HOBOT_LOG_WARN);
  } else if (log_level == "-e") {
    SetLogLevel(HOBOT_LOG_ERROR);
  } else if (log_level == "-f") {
    SetLogLevel(HOBOT_LOG_FATAL);
  } else {
    LOGE << "log option: [-i/-d/-w/-f] ";
    return 0;
  }

  auto vio_plg = std::make_shared<VioPlugin>(vio_config_file);
  auto smart_plg = std::make_shared<SmartPlugin>(smart_config_file);
  auto display_plg = std::make_shared<DisplayPlugin>(display_config_file);

  std::string can_plugin_config = "apa/configs/can_config.json";
  auto can_plg = std::make_shared<CanPlugin>(can_plugin_config);
  auto analysis_plg = std::make_shared<AnalysisPlugin>("config.json");
  auto can_generate_example_plugin = std::make_shared<CanExamplePlugin>();
  auto ret = can_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init can plugin";
    return -1;
  }

  ret = display_plg->Init();
  if (ret != 0) {
    LOGF << "display init failed";
    return -1;
  }

  ret = smart_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init smart plugin";
    return -1;
  }

  ret = vio_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init vio plugin";
    return -1;
  }

  can_generate_example_plugin->Init();
  analysis_plg->Init();
  can_generate_example_plugin->Start();

  ret = can_plg->Start();
  if (ret != 0) {
    LOGF << "can plugin start failed";
    return -1;
  }

  ret = display_plg->Start();
  if (ret != 0) {
    LOGF << "display start fail";
    return -1;
  }

  ret = smart_plg->Start();
  if (ret != 0) {
    LOGF << "smart plugin start failed";
    return -1;
  }
  analysis_plg->Start();
  sleep(1);
  ret = vio_plg->Start();
  if (ret != 0) {
    LOGF << "vio plugin start failed";
    return -1;
  }

  while (!exit_) {
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    // example: send can to
  }

  vio_plg->Stop();
  vio_plg->DeInit();
  vio_plg = nullptr;
  analysis_plg->Stop();
  smart_plg->Stop();
  smart_plg->DeInit();
  display_plg->Stop();
  display_plg->DeInit();
  return 0;
}
