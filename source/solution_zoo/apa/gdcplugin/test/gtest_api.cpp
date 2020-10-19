/* @Description: implement of gdcplugin test
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-07 20:33:54
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-07 20:38:23
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#include <sys/utsname.h>
#include <memory>
#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "gdcplugin/gdcplugin.h"

using horizon::vision::xproto::gdcplugin::GdcPlugin;
using GdcPluginPtr = std::shared_ptr<GdcPlugin>;

class GdcPluginTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}

  GdcPluginPtr gdcplugin = nullptr;
  std::mutex g_mtx_;
};

TEST_F(GdcPluginTest, plugin_apis) {
  int ret;
  SetLogLevel(INFO);
  gdcplugin = std::make_shared<GdcPlugin>("configs/gdcplugin_config.json");
  if (gdcplugin == NULL) {
    LOGE << "gdcplugin instance create failed";
    return;
  }

  ret = gdcplugin->Init();
  EXPECT_EQ(ret, 0);

  ret = gdcplugin->Start();
  EXPECT_EQ(ret, 0);

  sleep(10000);

  ret = gdcplugin->Stop();
  EXPECT_EQ(ret, 0);
}
