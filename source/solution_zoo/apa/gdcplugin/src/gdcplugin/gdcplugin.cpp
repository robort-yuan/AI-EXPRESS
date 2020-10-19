/*
 * @Description: implement of gdc plugin
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-04 20:30:10
 * @LastEditors: shiyu.fu@horizon.ai
 * @LastEditTime: 2020-09-04 20:30:10
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#include "gdcplugin/gdcplugin.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/json_key.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"

#include "xproto_msgtype/gdcplugin_data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace gdcplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::basic_msgtype::IpmImageMessage;

XPLUGIN_REGISTER_MSG_TYPE(XPLUGIN_IPM_MESSAGE)

void DumpIpmImage(std::vector<std::shared_ptr<CVImageFrame>> imgs,
                  std::vector<uint64_t> frame_ids, std::vector<int> chn_ids) {
  for (size_t i = 0; i < imgs.size(); ++i) {
    auto chn_id = chn_ids[i];
    auto frame_id = frame_ids[i];
    auto width = imgs[i]->Width();
    auto height = imgs[i]->Height();
    std::stringstream sstream;
    std::ofstream ofs;
    sstream << "./ipm_imgs/ipm_" << frame_id << "_" << chn_id << ".nv12";
    ofs.open(sstream.str(), std::ios_base::binary);
    ofs.write(reinterpret_cast<char *>(imgs[i]->img.data),
              (width * height * 3 / 2));
    ofs.close();
  }
}

GdcPlugin::GdcPlugin(const std::string &config_file) {
  config_file_ = config_file;
  LOGI << "gdcplugin config file:" << config_file_;
  Json::Value cfg_jv;
  std::ifstream infile(config_file_);
  infile >> cfg_jv;
  config_.reset(new JsonConfigWrapper(cfg_jv));
}

int GdcPlugin::Init() {
  all_in_one_vio_ = config_->GetBoolValue("all_in_one_vio");
  data_source_num_ = config_->GetIntValue("data_source_num");
  config_files_ = config_->GetSTDStringArray("config_files");
  concate_chns_ = config_->GetIntArray("concate");
  chn2direction_ = config_->GetIntArray("channel2direction");
  HOBOT_CHECK(!config_files_.empty()) << "Need to specifiy camera config file";
  HOBOT_CHECK(chn2direction_.size() == data_source_num_)
    << "Wrong channel2direction size";
  int ret = 0;
  if (config_files_.size() != 1) {
    HOBOT_CHECK(config_files_.size() == data_source_num_)
      << "#Config Files does not match #Data Sources";
    ret = ipm_util_.Init(config_files_[0], config_files_[1],
                         config_files_[2], config_files_[3],
                         chn2direction_, data_source_num_);
  } else {
    ret = ipm_util_.Init(config_files_[0], config_files_[0],
                         config_files_[0], config_files_[0],
                         chn2direction_, data_source_num_);
  }
  if (ret != 0) {
    LOGE << "Failed to init ipm sdk";
    return -1;
  }
  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&GdcPlugin::OnVioMessage, this, std::placeholders::_1));
  RegisterMsg(TYPE_MULTI_IMAGE_MESSAGE,
              std::bind(&GdcPlugin::OnVioMessage, this, std::placeholders::_1));
  return XPluginAsync::Init();
}

int GdcPlugin::Start() {
  LOGW << "GdcPlugin Start";
  return 0;
}

int GdcPlugin::Stop() {
  LOGW << "GDCPlugin Stop";
  return 0;
}

int GdcPlugin::OnVioMessage(XProtoMessagePtr msg) {
  auto vio_msg = std::static_pointer_cast<VioMessage>(msg);
  if (!all_in_one_vio_) {
    auto time_stamp = vio_msg->image_[0]->time_stamp;
    auto frame_id = vio_msg->image_[0]->frame_id;
    // ipm img's channel id = pym img's channel is + total channel num
    auto chn_id = vio_msg->image_[0]->channel_id + data_source_num_;
    std::vector<std::shared_ptr<CVImageFrame>> outputs;
    for (size_t idx = 0; idx < vio_msg->image_.size(); ++idx) {
      auto img_holder = std::make_shared<CVImageFrame>();
      outputs.push_back(img_holder);
    }

    int ret = ipm_util_.GenIPMImage(vio_msg->image_, outputs);
    if (ret != 0) {
      LOGE << "Failed to generate ipm image, " << ret;
      return ret;
    }

    std::vector<uint64_t> ttses;
    std::vector<uint64_t> frame_ids;
    std::vector<int> chn_ids;
    ttses.push_back(time_stamp);
    frame_ids.push_back(frame_id);
    chn_ids.push_back(chn_id);
    auto ipm_msg =
      std::make_shared<IpmImageMessage>(outputs, 1, ttses, frame_ids, chn_ids);
#ifdef DEBUG
    DumpIpmImage(outputs, frame_ids, chn_ids);
#endif
    PushMsg(ipm_msg);
    LOGI << "Pushed IpmImageMessage";
  } else {
    if (concate_chns_.empty()) {
      std::vector<std::shared_ptr<CVImageFrame>> outputs;
      for (size_t idx = 0; idx < vio_msg->image_.size(); ++idx) {
        auto img_holder = std::make_shared<CVImageFrame>();
        outputs.push_back(img_holder);
      }

      int ret = ipm_util_.GenIPMImage(vio_msg->image_, outputs);
      if (ret != 0) {
        LOGE << "Failed to generate ipm image, " << ret;
        return ret;
      }

      // send separately
      for (size_t i = 0; i < vio_msg->image_.size(); ++i) {
        std::vector<std::shared_ptr<CVImageFrame>> tmp_ipms;
        tmp_ipms.push_back(outputs[i]);
        std::vector<uint64_t> ttses;
        ttses.push_back(vio_msg->image_[i]->time_stamp);
        std::vector<uint64_t> frame_ids;
        frame_ids.push_back(vio_msg->image_[i]->frame_id);
        // ipm img's channel id = pym img's channel is + total channel num
        std::vector<int> chn_ids;
        chn_ids.push_back(vio_msg->image_[i]->channel_id + data_source_num_);
        auto ipm_msg =
          std::make_shared<IpmImageMessage>(tmp_ipms, 1, ttses,
                                            frame_ids, chn_ids);
#ifdef DEBUG
        DumpIpmImage(tmp_ipms, frame_ids, chn_ids);
#endif
        PushMsg(ipm_msg);
        LOGI << "Pushed IpmImageMessage";
      }
    } else {
      // concatenation, currently not support
    }
  }
  return 0;
}

}  // namespace gdcplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
