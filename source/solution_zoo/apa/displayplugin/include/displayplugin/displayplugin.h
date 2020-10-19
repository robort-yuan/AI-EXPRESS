/* @Description: displayplugin declaration
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-09 17:21:02
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-10 10:35:38
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#ifndef INCLUDE_DISPLAYPLUGIN_DISPLAYPLUGIN_H_
#define INCLUDE_DISPLAYPLUGIN_DISPLAYPLUGIN_H_

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <map>

#include "client_interface.h"  // NOLINT
#include "json/json.h"
#include "xproto/threads/threadpool.h"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "xproto_msgtype/vioplugin_data.h"
#include "horizon/vision_type/vision_type.hpp"
#include "media_codec/media_codec_manager.h"
#include "multivioplugin/viomessage.h"

#include "../../../common/data.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace displayplugin {

using hobot::vision::PymImageFrame;
using hobot::vision::CVImageFrame;
using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::basic_msgtype::SmartMessagePtr;
using horizon::vision::xproto::multivioplugin::ImageVioMessage;
using horizon::auto_client_adaptor::ImageDataPtr;
using horizon::auto_client_adaptor::ClientAdaptor;
using horizon::auto_client_adaptor::CameraChannelPtr;
using horizon::auto_client_adaptor::frame_data_t;
using horizon::vision::xproto::apa::ApaSmartResult;

struct ComparePym {
  bool operator()(const CameraChannelPtr &f1, const CameraChannelPtr &f2) {
    return (f1->param->frame_id > f2->param->frame_id);
  }
};

struct CompareSmart {
  bool operator()(const SmartMessagePtr m1, const SmartMessagePtr m2) {
    return (m1->frame_id > m2->frame_id);
  }
};

class DisplayPlugin : public XPluginAsync {
 public:
  DisplayPlugin() = default;
  ~DisplayPlugin() = default;
  explicit DisplayPlugin(const std::string &config_file);
  void SetConfig(const std::string &config_file) { config_file_ = config_file; }
  int Init() override;
  int Start() override;
  int Stop() override;

 private:
  int OnVioMessage(XProtoMessagePtr msg);
  int OnIpmMessage(XProtoMessagePtr msg);
  int OnSmartMessage(XProtoMessagePtr msg);
  int Reset();
  void EncodeJpeg(std::shared_ptr<ImageVioMessage> vio_msg);
  void MapSmartProc();
  void AssignSmartMessage(SmartMessagePtr msg, CameraChannelPtr frame);
  void PopBuffer(int channel_id);
  int SendClientMessage(std::shared_ptr<frame_data_t> one_frame);

 private:
  std::string config_file_;
  Json::Value cfg_jv_;
  int source_num_;
  std::string port_;
  std::map<int, std::priority_queue<CameraChannelPtr,
                                    std::vector<CameraChannelPtr>, ComparePym>>
    pym_img_buffer_;
  std::map<int, std::priority_queue<SmartMessagePtr,
                                    std::vector<SmartMessagePtr>, CompareSmart>>
    smart_msg_buffer_;
  std::shared_ptr<ClientAdaptor> adaptor_;
  hobot::CThreadPool jpeg_encode_thread_;
  hobot::CThreadPool data_send_thread_;
  std::mutex smart_mutex_;
  std::mutex video_mutex_;
  std::mutex map_mutex_;
  std::condition_variable map_smart_condition_;
  bool smart_stop_flag_;
  bool video_stop_flag_;
  bool map_stop_;
  std::shared_ptr<std::thread> worker_;
  // <channel_id, err_cnt>, max 100
  std::map<int, int> channel_err_cnt_;

  // media codec param
  uint8_t jpeg_quality_;
  uint8_t layer_;
  uint32_t image_width_, image_height_;
  int chn_;
  int frame_buf_depth_ = 0;
  int is_cbr_ = 1;
  int bitrate_ = 2000;
  int use_vb_ = 0;
  int org_img_width_ = 1280;
  int org_img_height_ = 720;
  int dst_img_width_ = 1280;
  int dst_img_height_ = 720;
  uint32_t cache_size_ = 25;
  int image_seg_type_;
  int ipm_seg_type_;
};

}  // namespace displayplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_DISPLAYPLUGIN_DISPLAYPLUGIN_H_
