/**
 * Copyright (c) 2018 Horizon Robotics. All rights reserved.
 * @brief provides base data struct for xstream framework
 * @file xstream_data.h
 * @author    chuanyi.yang
 * @email     chuanyi.yang@horizon.ai
 * @version   0.0.0.1
 * @date      2018.11.21
 */

#ifndef HOBOTXSDK_XSTREAM_DATA_H_
#define HOBOTXSDK_XSTREAM_DATA_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace xstream {

class CppContext;
class CContext;

/// Data State
enum class DataState {
  /// valid
  VALID = 0,
  /// filtered
  FILTERED = 1,
  /// invisible
  INVISIBLE = 2,
  /// disappeared
  DISAPPEARED = 3,
  /// invalid
  INVALID = 4,
};

/// base class of data structure.
/// The customer's data structure needs to inherit it.
struct BaseData {
  BaseData();
  virtual ~BaseData();
  /// type
  std::string type_ = "";
  /// name
  std::string name_ = "";
  /// error code
  int error_code_ = 0;
  /// error detail info
  std::string error_detail_ = "";
  /// context of C structure
  std::shared_ptr<CContext> c_data_;
  /// data status
  DataState state_ = DataState::VALID;
};

typedef std::shared_ptr<BaseData> BaseDataPtr;

struct BaseDataVector : public BaseData {
  BaseDataVector();

  std::vector<BaseDataPtr> datas_;
};

/// Wrap other data structures into derived classes of BaseData
template <typename Dtype>
struct XStreamData : public BaseData {
  Dtype value;
  XStreamData() {}
  explicit XStreamData(const Dtype &val) { value = val; }
};

/// param of inputdata
class InputParam {
 public:
  explicit InputParam(const std::string &unique_name) {
    unique_name_ = unique_name;
    is_json_format_ = false;
    is_enable_this_method_ = true;
  }
  virtual ~InputParam() = default;
  virtual std::string Format() = 0;

 public:
  bool is_json_format_;
  bool is_enable_this_method_;
  std::string unique_name_;
};

typedef std::shared_ptr<InputParam> InputParamPtr;

/**
 * \brief pre-difined param for method
 */
class DisableParam : public InputParam {
 public:
  enum class Mode {
    /// passthrough mode: use inputdata as outputdata;
    /// slot size of outputdata and inputdata must be equal;
    PassThrough,
    /// use pre-difined data as outputdata;
    /// slot size of predifine data and outputdata must be equal;
    UsePreDefine,
    /// set outputdata status as DataState::INVALID
    Invalid,
    /// flexibly passthrough mode
    /// 1. passthrough inputdata to outputdata in order
    /// 2. if slot size of inputdata greater than outputdata,
    ///    only use the first few slots.
    /// 3. if slot size of inputdata less than outputdata,
    ///    set status of last few slots invalid.
    BestEffortPassThrough
  };
  explicit DisableParam(const std::string &unique_name,
                        Mode mode = Mode::Invalid)
      : InputParam(unique_name), mode_{mode} {
    is_enable_this_method_ = false;
  }
  virtual ~DisableParam() = default;
  virtual std::string Format() { return unique_name_ + " : disabled"; }
  Mode mode_;
  /// pre-difined data, used in Mode::UsePreDefine
  std::vector<BaseDataPtr> pre_datas_;
};

typedef std::shared_ptr<DisableParam> DisableParamPtr;

/// json format parameter
class SdkCommParam : public InputParam {
 public:
  SdkCommParam(const std::string &unique_name, const std::string &param)
      : InputParam(unique_name) {
    param_ = param;
    is_json_format_ = true;
  }
  virtual std::string Format() { return param_; }
  virtual ~SdkCommParam() = default;

 public:
  std::string param_;  // json format
};
typedef std::shared_ptr<SdkCommParam> CommParamPtr;

/// InputData for XStreamSDK
struct InputData {
  /// input data, such as image, timestamp, box..
  std::vector<BaseDataPtr> datas_;
  /// InputParam
  std::vector<InputParamPtr> params_;
  /// input_data source id, used in multi-input; default value 0(single-input)
  uint32_t source_id_ = 0;
  /// context_ pasthroughed to OutputData::context_
  const void *context_ = nullptr;
};
typedef std::shared_ptr<InputData> InputDataPtr;

/// OutputData for XStreamSDK
struct OutputData {
  /// error code
  int error_code_ = 0;
  /// error info
  std::string error_detail_ = "";
  /// used in Node's outputdata, represents node's name
  std::string unique_name_ = "";
  /// outputdata name
  std::string output_type_ = "";
  /// output data, such as detection_box, landmarks..
  std::vector<BaseDataPtr> datas_;
  /// passthroughed from inputdata
  const void *context_ = nullptr;
  /// sequence_id in single-input
  int64_t sequence_id_ = 0;
  /// input_data source id, used in multi-input; default value 0(single-input)
  uint32_t source_id_ = 0;
  uint64_t global_sequence_id_ = 0;
};
typedef std::shared_ptr<OutputData> OutputDataPtr;

/// callback func
typedef std::function<void(OutputDataPtr)> XStreamCallback;

}  // namespace xstream

#endif  // HOBOTXSDK_XSTREAM_DATA_H_
