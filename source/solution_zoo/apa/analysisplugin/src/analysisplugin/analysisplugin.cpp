/*
 * @Description: implement of analysis plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-14 15:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-09-14 15:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#include "analysisplugin/analysisplugin.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"
#include "horizon/vision_type/vision_type.hpp"
#include "../../canplugin/include/canplugin/can_data_type.h"


using hobot::vision::PymImageFrame;
using horizon::vision::xproto::canplugin::can_header;
using horizon::vision::xproto::canplugin::can_frame;
using horizon::vision::xproto::canplugin::can_message;

namespace horizon {
namespace vision {
namespace xproto {
namespace analysisplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using horizon::vision::xproto::multivioplugin::MultiVioMessage;

AnalysisPlugin::AnalysisPlugin(const std::string &config_file) {
  config_file_ = config_file;
  // 可能需要做些其他初始化的工作
}

int AnalysisPlugin::Init() {
  // 可能需要做的工作有如下：
  // 1）解析配置文件
  // 2）初始化ipm模块
  // 3) 初始化智能分析workflow模块
  // 4）初始化展示客户端相关初始化代码


  // 绑定消息
  RegisterMsg(TYPE_MULTI_IMAGE_MESSAGE,
              std::bind(&AnalysisPlugin::FeedMultiVio,
                    this, std::placeholders::_1));
  RegisterMsg(TYPE_CAN_BUS_FROM_MCU_MESSAGE,
              std::bind(&AnalysisPlugin::FeedCanFromMcu, this,
                        std::placeholders::_1));

  // 最后一定要调用XPluginAsync::Init()
  return XPluginAsync::Init();
}

int AnalysisPlugin::FeedMultiVio(XProtoMessagePtr msg) {
  auto multi_frame = std::static_pointer_cast<MultiVioMessage>(msg);

  // 解析图像数据
  // 1个MultiVioMessage包含4个ImageVioMessage;
  // 1个ImageVioMessage包含1个PymImageFrame，
  // 1个PymImageFrame就是对应一路原图图像的金字塔图像
  for (size_t m = 0; m < multi_frame->multi_vio_img_.size(); m++) {
    std::shared_ptr<ImageVioMessage> valid_frame =
        multi_frame->multi_vio_img_[m];
    assert(valid_frame->image_.size() == 1);
    std::shared_ptr<PymImageFrame> one_image = valid_frame->image_[0];
    int channel_id = one_image->channel_id;
    int frame_id = one_image->frame_id;
    uint64_t timestamp = one_image->time_stamp;
    int width = one_image->down_scale[0].width;
    int height = one_image->down_scale[0].height;
    int stride = one_image->down_scale[0].stride;
    uint8_t *y_data =
        reinterpret_cast<uint8_t *>(one_image->down_scale[0].y_vaddr);
    uint8_t *uv_data =
        reinterpret_cast<uint8_t *>(one_image->down_scale[0].c_vaddr);
    LOGI << "channel_id:" << channel_id << ", frame_id:" << frame_id
         << ", timestamp:" << timestamp << ", width:" << width
         << ", height:" << height << ", stride:" << stride;
    if (nullptr == y_data) {
      LOGF << "y_data is nullptr";
    }
    if (nullptr == uv_data) {
      LOGF << "uv_data is nullptr";
    }
  }
  // 图像格式转换
  // 拼接图像
  // 模型预测
  // 输出感知结果到xproto总线，感知结果类型需要再定义
  return 0;
}

int AnalysisPlugin::FeedCanFromMcu(XProtoMessagePtr msg) {
  auto can_msg = std::static_pointer_cast<CanBusFromMcuMessage>(msg);
  if (can_msg->can_data_len_ <= 0) {
    LOGF << "receive empty can data from mcu";
    return -1;
  }
  can_header *header = nullptr;
  can_frame *frame = nullptr;
  header = reinterpret_cast<struct can_header *>(&(can_msg->can_data_[0]));
  LOGI << "ts: " << header->time_stamp;
  LOGI << "rc: " << static_cast<int>(header->counter);
  LOGI << "number: " << static_cast<int>(header->frame_num);
  frame =
      reinterpret_cast<can_frame *>(can_msg->can_data_ + sizeof(can_header));
  for (int i = 0; i < header->frame_num; ++i) {
    LOGI << "can_id: " << frame[i].can_id
         << " dlc: " << static_cast<int>(frame[i].can_dlc) << " data:";
    for (int j = 0; j < 8; j++) {
      LOGI << " " << static_cast<int>(frame[i].data[j]);
    }
  }
  // 可能需要将can放入一个buffer，用于和图像/感知结果进行融合
  return 0;
}

int AnalysisPlugin::Start() {
  LOGW << "AnalysisPlugin Start";
  return 0;
}

int AnalysisPlugin::Stop() {
  LOGW << "AnalysisPlugin Stop";
  return 0;
}

}  // namespace analysisplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
