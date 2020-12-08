/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: Songshan Gong
 * @Mail: songshan.gong@horizon.ai
 * @Date: 2019-08-04 02:41:22
 * @Version: v0.0.1
 * @Brief: smartplugin declaration
 * @Last Modified by: Songshan Gong
 * @Last Modified time: 2019-09-30 00:45:01
 */

#ifndef INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_
#define INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "xproto/msgtype/include/xproto_msgtype/protobuf/x3.pb.h"

#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "smartplugin/runtime_monitor.h"
#include "smartplugin/smart_config.h"
#include "smartplugin/traffic_info.h"
#include "xproto_msgtype/smartplugin_data.h"


namespace horizon {
namespace vision {
namespace xproto {
namespace smartplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;

using xstream::InputDataPtr;
using xstream::OutputDataPtr;
using xstream::XStreamSDK;
using horizon::vision::xproto::basic_msgtype::SmartMessage;

struct VehicleSmartMessage : SmartMessage {
 public:
  VehicleSmartMessage();
  std::string Serialize() override;
  std::string Serialize(int ori_w, int ori_h, int dst_w, int dst_h);
  void Serialize_Print(Json::Value &root);
  void Serialize_Dump_Result();
 public:
  int camera_type;
  std::vector<VehicleInfo> vehicle_infos;
  std::vector<PersonInfo> person_infos;
  std::vector<NoMotorVehicleInfo> nomotor_infos;
  std::vector<VehicleCapture> capture_infos;
  std::vector<uint64_t> lost_track_ids;
  std::vector<AnomalyInfo> anomaly_infos;

  std::vector<TrafficConditionInfo> traffic_condition_infos;
  TrafficStatisticsInfo traffic_statistics_info;
};

struct target_key {
  std::string category;
  int id;
  target_key(std::string category, int id) {
    this->category = category;
    this->id = id;
  }
};

struct cmp_key {
  bool operator()(const target_key &a, const target_key &b) {
    if (a.category == b.category) {
      return a.id < b.id;
    }
    return a.category < b.category;
  }
};

struct CustomSmartMessage : SmartMessage {
  explicit CustomSmartMessage(
    xstream::OutputDataPtr out) : smart_result(out) {
    type_ = TYPE_SMART_MESSAGE;
  }
  std::string Serialize() override;
  std::string Serialize(int ori_w, int ori_h, int dst_w, int dst_h) override;
  void Serialize_Print(Json::Value &root);
  void SetAPMode(bool ap_mode) {
    ap_mode_ = ap_mode;
  }
  void Serialize_Dump_Result();

 protected:
  xstream::OutputDataPtr smart_result;
  bool ap_mode_ = false;

 private:
  static std::map<int, int> fall_state_;
  static std::mutex fall_mutex_;
  static std::map<int, int> gesture_state_;
  static std::map<int, float> gesture_start_time_;
};

class SmartPlugin : public XPluginAsync {
 public:
  SmartPlugin() = default;
  explicit SmartPlugin(const std::string& config_file);

  void SetConfig(const std::string& config_file) { config_file_ = config_file; }

  ~SmartPlugin() = default;
  int Init() override;
  int DeInit() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "SmartPlugin"; }

 private:
  int Feed(XProtoMessagePtr msg);
  void OnCallback(xstream::OutputDataPtr out);
  void ParseConfig();

  std::shared_ptr<XStreamSDK> sdk_;
  int sdk_monitor_interval_;
  std::string config_file_;
  std::shared_ptr<RuntimeMonitor> monitor_;
  std::shared_ptr<JsonConfigWrapper> config_;
  std::string xstream_workflow_cfg_file_;
  bool enable_profile_{false};
  std::string profile_log_file_;
  bool result_to_json_{false};
  bool dump_result_{false};
  Json::Value root_;
  bool run_flag_ = false;
};

}  // namespace smartplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_SMARTPLUGIN_SMARTPLUGIN_H_
