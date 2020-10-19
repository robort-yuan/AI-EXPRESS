/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:30:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */
#ifndef INCLUDE_MULTIVIOPLUGIN_VIOPLUGIN_H_
#define INCLUDE_MULTIVIOPLUGIN_VIOPLUGIN_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <map>
#include <condition_variable>
#include "json/json.h"

#include "horizon/vision_type/vision_type.hpp"
#include "hobot_vision/blocking_queue.hpp"
#include "xproto/plugin/xpluginasync.h"

#include "multivioplugin/vioconfig.h"
#include "multivioplugin/vioproduce.h"

#include "opencv2/opencv.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

class VioPlugin : public xproto::XPluginAsync {
 public:
  VioPlugin() = delete;
  explicit VioPlugin(const std::string &path);
  ~VioPlugin() override;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "MultiVioPlugin"; }
  bool IsInited() { return is_inited_; }

 private:
  VioConfig *GetConfigFromFile(const std::string &path);

  void SyncPymImage();
  void SyncPymImages();

 private:
  VioConfig *config_;
  std::vector<std::shared_ptr<VioProduce>> vio_produce_handle_;
  bool is_inited_ = false;
  // image_buffer_ used to cache pymrid buffer
  std::vector<hobot::vision::BlockingQueue<std::shared_ptr<VioMessage>> *>
      pym_img_buffer_;
  uint32_t max_buffer_size_;  // max buffer size for each channel
  // first = channel index, second = 0: can send, 1 : can not send
  std::map<int, int> channel_status_;
  std::mutex pym_img_mutex_;
  uint32_t is_msg_package_;
  std::vector<int> chn_grp_;
};

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif
