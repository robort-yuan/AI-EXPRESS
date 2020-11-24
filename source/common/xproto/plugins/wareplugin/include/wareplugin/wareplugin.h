/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail:
 * @Date: 2020-11-02 20:38:52
 * @Version: v0.0.1
 * @Last Modified by:
 * @Last Modified time: 2020-11-02 20:38:52
 */

#include <string>
#include <memory>
#include "smartplugin/smartplugin.h"
#include "wareplugin/waredb.h"
#include "wareplugin/utils/jsonConfigWrapper.h"

#ifndef INCLUDE_WAREPLUGIN_WAREPLUGIN_H_
#define INCLUDE_WAREPLUGIN_WAREPLUGIN_H_
namespace horizon {
namespace vision {
namespace xproto {
namespace wareplugin {

using SnapshotInfoXStreamBaseData =
hobot::vision::SnapshotInfo<xstream::BaseDataPtr>;
using SnapshotInfoXStreamBaseDataPtr =
std::shared_ptr<SnapshotInfoXStreamBaseData>;
using XStreamSnapshotInfo =
xstream::XStreamData<SnapshotInfoXStreamBaseDataPtr>;
using XStreamSnapshotInfoPtr = std::shared_ptr<XStreamSnapshotInfo>;

struct SnapSmartMessage : public smartplugin::CustomSmartMessage {
  explicit SnapSmartMessage(
      xstream::OutputDataPtr out) : CustomSmartMessage(out) {
  }
  explicit SnapSmartMessage(const CustomSmartMessage& smart_message) :
  CustomSmartMessage(smart_message) { }

  std::string Serialize(int ori_w, int ori_h, int dst_w, int dst_h) override;

  void SetRecognizeMode(bool is_recognize) {
    is_recognize_ = is_recognize;
  }
  void SetAddRecordMode(bool is_add_record) {
    is_add_record_ = is_add_record;
  }

 private:
  void ConvertSnapShotOutput(
      const xstream::BaseDataVector *targets_snapshots,
      const xstream::BaseDataVector *targets_face_features,
      float x_ratio, float y_ratio,
      x3::CaptureFrameMessage *capture_msg);
  void ConvertTargetInfo(const xstream::BaseDataVector *target_snaps,
                         const xstream::BaseDataVector *target_features,
                         float x_ratio, float y_ratio,
                         x3::CaptureTarget *capture_targets);
  void ConvertRecognize(const xstream::BaseDataPtr face_feature,
                        x3::DBResult *capture_dbresult);
  void ConvertSnapShotInfo(SnapshotInfoXStreamBaseDataPtr sp_snapshot,
                           const xstream::BaseDataPtr face_feature,
                           float x_ratio, float y_ratio,
                           x3::Capture *capture_target);

 private:
  bool is_recognize_ = false;
  bool is_add_record_ = false;
};

class WarePlugin : public XPluginAsync {
 public:
  WarePlugin() = default;
  explicit WarePlugin(const std::string& config_file);

  ~WarePlugin() = default;
  int Init() override;
  int Start() override;
  int Stop() override;
  std::string desc() const { return "WarePlugin"; }

 private:
  int OnGetSmarterResult(const XProtoMessagePtr& msg);
  void ParseConfig();

  std::string config_file_;
  std::shared_ptr<JsonConfigWrapper> config_;

 private:
  bool is_recognize_ = false;
  bool is_add_record_ = false;

  bool isinit_ = false;
};

}   //  namespace wareplugin
}   //  namespace xproto
}   //  namespace vision
}   //  namespace horizon

#endif  //  INCLUDE_WAREPLUGIN_WAREPLUGIN_H_
