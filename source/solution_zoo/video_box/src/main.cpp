/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */

#include <signal.h>
#include <unistd.h>
#include <malloc.h>
#include <iostream>
#include <sstream>
#include <memory>
// #include "hbipcplugin/hbipcplugin.h"
#include "hobotlog/hobotlog.hpp"
#include "rtspplugin/rtspplugin.h"
#include "smartplugin_box/smartplugin.h"
#include "vioplugin/vioplugin.h"
#include "visualplugin/visualplugin.h"
#include "bpu_predict/bpu_predict_extension.h"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using std::chrono::seconds;

// using horizon::vision::xproto::hbipcplugin::HbipcPlugin;
using horizon::vision::xproto::rtspplugin::RtspPlugin;
using horizon::vision::xproto::smartplugin_multiplebox::SmartPlugin;
using horizon::vision::xproto::vioplugin::VioPlugin;
using horizon::vision::xproto::visualplugin::VisualPlugin;
static bool exit_ = false;

static void signal_handle(int param) {
  std::cout << "recv signal " << param << ", stop" << std::endl;
  if (param == SIGINT) {
    exit_ = true;
  }
  if (param == SIGSEGV) {
    std::cout << "recv segment fault, exit exe, maybe need reboot..."
              << std::endl;
    exit(-1);
  }
}

int main(int argc, char **argv) {
  mallopt(M_TRIM_THRESHOLD, 128 * 1024);
  HB_BPU_setGlobalConfig(BPU_GLOBAL_ENGINE_TYPE, "native");
  std::string run_mode = "ut";

  if (argc < 4) {
    std::cout << "Usage: smart_main vio_config_file "
              << "xstream_config_file visualplugin_config "
              << "[-i/-d/-w/-f] " << std::endl;
    return 0;
  }
  std::string box_config_file = std::string(argv[1]);
  std::string visual_config_file = std::string(argv[2]);

  std::string log_level(argv[3]);
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

  if (argc == 5) {
    run_mode.assign(argv[4]);
    if (run_mode != "ut" && run_mode != "normal") {
      LOGE << "not support mode: " << run_mode;
      return 0;
    }
  }

  signal(SIGINT, signal_handle);
  signal(SIGPIPE, signal_handle);

  auto visual_plg = std::make_shared<VisualPlugin>(visual_config_file);
  auto rtsp_plg = std::make_shared<RtspPlugin>(box_config_file);

  if (visual_plg)
    visual_plg->Init();

  if (visual_plg)
    visual_plg->Start();

  sleep(5);

  rtsp_plg->Init();
  auto ret = rtsp_plg->Start();
  if (ret < 0) {
    LOGE << "Failed to start rtsp plugin ret:" << ret;
    rtsp_plg->Stop();
    if (-2 == ret) {
      LOGE << "ERROR!!! open rtsp fail, please check url";
    }
    return 0;
  } else {
    LOGI << "rtsp plugin start success";
  }

  auto smart_plg = std::make_shared<SmartPlugin>(box_config_file);
  ret = smart_plg->Init();
  if (ret != 0) {
    LOGE << "Failed to init smart plugin";
    return 2;
  }
  smart_plg->Start();

  if (run_mode == "ut") {
    std::this_thread::sleep_for(std::chrono::seconds(30));
  } else {
    while (!exit_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(40));
      LOGD << "wait to quit";
    }
  }

  rtsp_plg->Stop();
  smart_plg->Stop();
  if (visual_plg)
     visual_plg->Stop();

  rtsp_plg->DeInit();
  smart_plg->DeInit();
  visual_plg->DeInit();

  return 0;
}
