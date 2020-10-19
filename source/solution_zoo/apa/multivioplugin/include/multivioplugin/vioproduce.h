/*
 * @Description: implement of vioplugin by J3 media lib
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-08-22 16:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-08-26 18:30:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_MULTIVIOPLUGIN_VIOPRODUCE_H_
#define INCLUDE_MULTIVIOPLUGIN_VIOPRODUCE_H_
#include <atomic>
#include <cstddef>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <memory>
#include <vector>
#include <unordered_map>

#include "json/json.h"

#include "multivioplugin/viomessage.h"
#include "multivioplugin/vioconfig.h"
#include "multivioplugin/executor.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace multivioplugin {

class VioProduce : public std::enable_shared_from_this<VioProduce> {
 public:
  static std::shared_ptr<VioProduce>
    CreateVioProduce(const std::string &data_source);

  virtual ~VioProduce() {}

  // called by vioplugin, add a run job to executor, async invoke, thread pool
  int Start();

  int Stop();

  // produce inputs, subclass must be implement
  virtual int Run() = 0;

  // finish producing inputs，common use
  virtual int Finish();

  // set VioConfig
  int SetConfig(VioConfig *config, int index);

  // viomessage can be base class
  using Listener = std::function<int(const std::shared_ptr<VioMessage> &input)>;

  // set callback function
  int SetListener(const Listener &callback);

  virtual void FreeBuffer();

  bool AllocBuffer();

  virtual void WaitUntilAllDone();

 protected:
  VioProduce() : is_running_{false} {
    auto config = VioConfig::GetConfig();
    auto json = config->GetJson();
    max_vio_buffer_ = json["max_vio_buffer"].asUInt();
    std::atomic_init(&consumed_vio_buffers_, 0);
  }

  explicit VioProduce(int max_vio_buffer) : is_running_{false} {
    max_vio_buffer_ = max_vio_buffer;
    std::atomic_init(&consumed_vio_buffers_, 0);
  }

 protected:
  VioConfig *config_ = nullptr;
  int index_;  // camera/pipeline index, start from 0
  std::function<int(const std::shared_ptr<VioMessage> &input)> push_data_cb_ =
      nullptr;
  std::atomic<bool> is_running_;
  std::atomic<int> consumed_vio_buffers_;
  int max_vio_buffer_ = 0;
  std::future<bool> task_future_;
  std::string cam_type_ = "quad";
  enum class TSTYPE {
    RAW_TS,    // 读取pvio_image->timestamp
    FRAME_ID,  // 读取pvio_image->frame_id
    INPUT_CODED  // 解析金字塔0层y图的前16个字节，其中编码了timestamp。
  };
  TSTYPE ts_type_ = TSTYPE::RAW_TS;
  static const std::unordered_map<std::string, TSTYPE> str2ts_type_;
};

class VioCamera : public VioProduce {
 public:
  int Run() override;

 protected:
  VioCamera() = default;
  virtual ~VioCamera() = default;

 protected:
  uint32_t sample_freq_ = 1;

 private:
  int ReadTimestamp(void *addr, uint64_t *timestamp);
};

class PanelCamera : public VioCamera {
 public:
  static std::mutex vio_mutex_;
  static bool vio_init_;
  explicit PanelCamera(const std::string &vio_cfg_file);
  virtual ~PanelCamera();

 private:
  int camera_index_ = -1;
  int camera_num_ = 0;
};

}  // namespace multivioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VIOPLUGIN_VIOPRODUCE_H_
