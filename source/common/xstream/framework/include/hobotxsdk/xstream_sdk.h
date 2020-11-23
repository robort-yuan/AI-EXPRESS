/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief     provides xstream framework interface
 * @file      xstream_sdk.h
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */
#ifndef HOBOTXSDK_XSTREAM_SDK_H_
#define HOBOTXSDK_XSTREAM_SDK_H_

#include <string>
#include <vector>

#include "xstream_data.h"

namespace xstream {

/**
 * Example Usage:
 * xstream::XStreamSDK *flow = xstream::XStreamSDK::CreateSDK();
 * flow->SetConfig("config_file", config);
 * flow->Init();
 * InputDataPtr inputdata(new InputData());
 * // ... construct inputdata
 * auto out = flow->SyncPredict(inputdata);
 * // PrintOut(out);
 * // ... handle outputdata
 * delete flow;
 */

/// XStream API
class XStreamSDK {
 public:
  /// Base class(derived class need to be defined, such as XStreamFlow)
  virtual ~XStreamSDK() {}
  /// creat SDK instance
  static XStreamSDK *CreateSDK();

  /// key："config_file", value: workflow config file(json) path
  virtual int SetConfig(
      const std::string &key,
      const std::string &value) = 0;
  /// Init workflow
  virtual int Init() = 0;
  /// Update InputParam for specified method instance
  virtual int UpdateConfig(const std::string &unique_name,
                           InputParamPtr ptr) = 0;
  /// Get InputParam of specified method
  virtual InputParamPtr GetConfig(const std::string &unique_name) const = 0;
  /// Get Version of specified method
  virtual std::string GetVersion(const std::string &unique_name) const = 0;
  /// SyncPredict Func for SingleOutput mode
  virtual OutputDataPtr SyncPredict(InputDataPtr input) = 0;
  /// SyncPredict Func for MultiOutput mode
  virtual std::vector<OutputDataPtr> SyncPredict2(InputDataPtr input) = 0;

  /**
   *  Set callback func, only support async mode
   *
   * Note: XStreamSDK should be inited(Init()) before SetCallback()
   * @param callback [in], callback func
   * @param name [in], workflow node name;
   * name = "": default param, set callback for final outputdata;
   * name: node name, set callback for specifed node, callback func handle the node's outputdata
   */
  virtual int SetCallback(XStreamCallback callback,
                          const std::string &name = "") = 0;
  /// AsyncPredict Func
  virtual int64_t AsyncPredict(InputDataPtr input) = 0;

  virtual int64_t GetTaskNum() = 0;
};

}  // namespace xstream

#endif  // HOBOTXSDK_XSTREAM_SDK_H_
