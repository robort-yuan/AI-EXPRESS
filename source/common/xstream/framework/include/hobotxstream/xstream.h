/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework interface
 * @file      xstream.h
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXSTREAM_XSTREAM_H_
#define HOBOTXSTREAM_XSTREAM_H_

#include <mutex>
#include <string>
#include <vector>
#include <memory>
#if defined(HOBOTSDK_ENGINE)
#include "hobotxstream/hobotsdk/schedulerV2.h"
#else
#include "hobotxstream/scheduler.h"
#endif
#include <unordered_map>
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_sdk.h"
#include "hobotxstream/method_factory.h"

namespace xstream {
/// XStreamFlow API
class XStreamFlow : public XStreamSDK {
 public:
  XStreamFlow();
  virtual ~XStreamFlow();

 public:
  virtual int SetConfig(
      const std::string &key,
      const std::string &value);  // set config file
  int SetCallback(XStreamCallback callback,
                  const std::string &name) override;  // set callback func
  virtual int UpdateConfig(const std::string &unique_name,
                           InputParamPtr param_ptr);
  InputParamPtr GetConfig(const std::string &unique_name) const override;
  std::string GetVersion(const std::string &unique_name) const override;
  virtual int Init();
  int Init(MethodFactoryPtr method_factory);
  // SyncPredict Func for SingleOutput mode
  OutputDataPtr SyncPredict(InputDataPtr input) override;
  // SyncPredict Func for MultiOutput mode
  std::vector<OutputDataPtr> SyncPredict2(InputDataPtr input) override;
  // AsyncPredict Func
  int64_t AsyncPredict(InputDataPtr input) override;

  // Get current task_num
  int64_t GetTaskNum();
 private:
  OutputDataPtr OnError(int64_t error_code, const std::string &error_detail);

 private:
#if defined(HOBOTSDK_ENGINE)
  SchedulerV2Ptr scheduler_;
#else
  SchedulerPtr scheduler_;
#endif
  XStreamCallback callback_;
  std::string config_file_;
  std::mutex mutex_;
  bool is_initial_;
  std::unordered_map<std::string, std::string> param_dict_;
  ProfilerPtr profiler_;
};

}  // namespace xstream

#endif  // HOBOTXSTREAM_XSTREAM_H_
