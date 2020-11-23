/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     Method interface of xstream framework
 * @file method.h
 * @author    shuhuan.sun
 * @email     shuhuan.sun@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXSTREAM_METHOD_H_
#define HOBOTXSTREAM_METHOD_H_

#include <memory>
#include <string>
#include <vector>

#include "hobotxsdk/xstream_data.h"

namespace xstream {
/// Method Info
struct MethodInfo {
  /// is thread safe
  bool is_thread_safe_ = false;
  /// is need reorder, the order of outputdata must be same as the inputdata
  bool is_need_reorder = false;
  /// is dependent on inputdata source
  bool is_src_ctx_dept = false;
};

class Method {
 public:
  virtual ~Method();
  /// Init
  virtual int Init(const std::string &config_file_path) = 0;
  // overload Init
  virtual int InitFromJsonString(const std::string &config) { return -1; }
  /// Update Method Parameter
  virtual int UpdateParameter(InputParamPtr ptr) = 0;
  // Process Func
  // <parameter> input: input data, input[i][j]: batch i, slot j
  virtual std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<InputParamPtr> &param) = 0;
  /// grt Method Parameter
  virtual InputParamPtr GetParameter() const = 0;
  /// get Method Version
  virtual std::string GetVersion() const = 0;
  /// destructor
  virtual void Finalize() = 0;
  /// get MethodInfo
  virtual MethodInfo GetMethodInfo();
  /// change Profiler status
  virtual void OnProfilerChanged(bool on) = 0;
};

typedef std::shared_ptr<Method> MethodPtr;

}  // namespace xstream

#endif  // HOBOTXSTREAM_METHOD_H_
