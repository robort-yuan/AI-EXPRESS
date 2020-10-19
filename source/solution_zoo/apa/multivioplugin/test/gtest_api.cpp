/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include <sys/utsname.h>
#include <memory>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "multivioplugin/vioplugin.h"
#include "multivioplugin/viomessage.h"
#include "opencv2/opencv.hpp"


using horizon::vision::xproto::multivioplugin::VioPlugin;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::multivioplugin::MultiVioMessage;
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessagePtr;
using VioPluginPtr = std::shared_ptr<VioPlugin>;

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_MULTI_IMAGE_MESSAGE)

class MultiImageTestPlugin : public XPluginAsync{
 public:
  MultiImageTestPlugin() = default;
  ~MultiImageTestPlugin() = default;

  int Init() {
    LOGI << "register multimage ";
    RegisterMsg(TYPE_MULTI_IMAGE_MESSAGE,
                  std::bind(&MultiImageTestPlugin::DumpMultiImage,
                  this, std::placeholders::_1));

    XPluginAsync::Init();
    return 0;
  }

  int DumpMultiImage(XProtoMessagePtr msg);
};

using MultiImageTestPluginPtr = std::shared_ptr<MultiImageTestPlugin>;

class VioPluginTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  void TearDown() override {}

  VioPluginPtr vioplugin = nullptr;
  MultiImageTestPluginPtr multiImageTestPlugin = nullptr;
  std::mutex g_mtx_;
};

int MultiImageTestPlugin::DumpMultiImage(XProtoMessagePtr msg) {
  LOGI << "DumpMultiImage";
  auto multi_frame = std::dynamic_pointer_cast<MultiVioMessage>(msg);
  cv::Mat yuv_img;
  std::vector<uchar> img_buf;
  img_buf.clear();
  img_buf.reserve(500*1024);

  for (size_t i = 0; i < multi_frame->multi_vio_img_.size(); i++) {
    auto frame = multi_frame->multi_vio_img_[i];
    VioMessage *vio_msg = frame.get();
    auto pym_image = vio_msg->image_[0];
    auto height = pym_image->down_scale[0].height;
    auto width = pym_image->down_scale[0].width;
    auto y_addr = pym_image->down_scale[0].y_vaddr;
    auto uv_addr = pym_image->down_scale[0].c_vaddr;
    auto img_y_size = height * pym_image->down_scale[0].stride;
    auto img_uv_size = img_y_size / 2;

    yuv_img.create(height * 3 / 2, width, CV_8UC1);
    memcpy(yuv_img.data, reinterpret_cast<uint8_t*>(y_addr), img_y_size);
    memcpy(yuv_img.data + height * width, reinterpret_cast<uint8_t*>
        (uv_addr), img_uv_size);

    std::vector<int> params;
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(50);
    cv::Mat img;
    cv::cvtColor(yuv_img, img, cv::COLOR_YUV2BGR_NV12);
    cv::imencode(".jpg", img, img_buf, params);

    int frame_id = pym_image->frame_id;
    int chn = pym_image->channel_id;
    std::string file_name = "pyr_images/out_stream_" + std::to_string(chn)
          + "_" + std::to_string(frame_id++) + ".jpg";
    std::fstream fout(file_name, std::ios::out | std::ios::binary);
    fout.write((const char *)img_buf.data(), img_buf.size());
    fout.close();
  }

  return 0;
}

TEST_F(VioPluginTest, vio_camera) {
  int ret;
  SetLogLevel(INFO);
  vioplugin = std::make_shared<VioPlugin>("configs/vio_config.json.j3dev");
  if (vioplugin == NULL) {
    LOGE << "vioplugin instance create failed";
    return;
  }

  multiImageTestPlugin = std::make_shared<MultiImageTestPlugin>();
  if (multiImageTestPlugin == NULL) {
    LOGE << "multiImageTestPlugin instance create failed";
    return;
  }

  ret = vioplugin->Init();
  EXPECT_EQ(ret, 0);

  ret = multiImageTestPlugin->Init();
  EXPECT_EQ(ret, 1);

  ret = vioplugin->Start();
  EXPECT_EQ(ret, 0);

  sleep(60000);

  ret = vioplugin->Stop();
  EXPECT_EQ(ret, 0);
}
