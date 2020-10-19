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

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "json/json.h"

#include "hobotlog/hobotlog.hpp"
#include "multismartplugin/smartplugin.h"
#include "multivioplugin/vioplugin.h"
#include "multiwebsocketplugin/websocketplugin.h"
#include "gdcplugin/gdcplugin.h"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using std::chrono::seconds;

using horizon::vision::xproto::multismartplugin::SmartPlugin;
using horizon::vision::xproto::multivioplugin::VioPlugin;
using horizon::vision::xproto::multiwebsocketplugin::WebsocketPlugin;
using horizon::vision::xproto::gdcplugin::GdcPlugin;

static bool exit_ = false;

static void signal_handle(int param) {
  std::cout << "recv signal " << param << ", stop" << std::endl;
  if (param == SIGINT) {
    exit_ = true;
  }
}

int main(int argc, char **argv) {
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

  std::string log_level(argv[5]);
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

  signal(SIGINT, signal_handle);
  signal(SIGPIPE, signal_handle);
  signal(SIGSEGV, signal_handle);

  auto vio_plg = std::make_shared<VioPlugin>(vio_config_file);
  auto gdc_plg = std::make_shared<GdcPlugin>(gdc_config_file);
  auto smart_plg = std::make_shared<SmartPlugin>(smart_config_file);
  auto output_plg = std::make_shared<WebsocketPlugin>(websocket_config_file);
  auto ret = output_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to websocket plugin";
    return -1;
  }

  ret = smart_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init vio";
    return -1;
  }

  ret = vio_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init vio";
    return -1;
  }

  ret = gdc_plg->Init();
  if (ret != 0) {
    LOGF << "Failed to init gdcplugin";
    return -1;
  }

  ret = output_plg->Start();
  if (ret != 0) {
    LOGF << "websocket plugin start failed";
    return -1;
  }
  ret = smart_plg->Start();
  if (ret != 0) {
    LOGF << "smart plugin start failed";
    return -1;
  }
  sleep(1);
  ret = vio_plg->Start();
  if (ret != 0) {
    LOGF << "vio plugin start failed";
    return -1;
  }

  ret = gdc_plg->Start();
  if (ret != 0) {
    LOGF << "gdcplugin start failed";
    return -1;
  }

  while (!exit_) {
    std::this_thread::sleep_for(std::chrono::microseconds(40));
  }

  vio_plg->Stop();
  vio_plg->DeInit();
  vio_plg = nullptr;
  gdc_plg->Stop();
  gdc_plg->DeInit();
  smart_plg->Stop();
  smart_plg->DeInit();
  output_plg->Stop();
  output_plg->DeInit();
  return 0;
}
