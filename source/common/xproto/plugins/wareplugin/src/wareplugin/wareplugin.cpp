/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail:
 * @Date:
 * @Version: v0.0.1
 * @Brief:
 * @Last Modified by:
 * @Last Modified time:
 */

#include "wareplugin/wareplugin.h"
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "horizon/vision_type/vision_type_util.h"
#include "mcplugin/mcmessage.h"
#include "smartplugin/smartplugin.h"
#include "utils/executor.hpp"
#include "utils/votmodule.h"
#include "uvcplugin/uvcplugin.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto_msgtype/protobuf/x3.pb.h"
#include "wareplugin/utils/util.h"
namespace horizon {
namespace vision {
namespace xproto {
namespace wareplugin {

WarePlugin::WarePlugin(const std::string& config_file) {
  config_file_ = config_file;
  LOGI << "WarePlugin config file:" << config_file_;
  Json::Value cfg_jv;
  std::ifstream infile(config_file_);
  if (infile) {
    infile >> cfg_jv;
    config_.reset(new JsonConfigWrapper(cfg_jv));
    ParseConfig();
  } else {
    LOGE << "open WarePlugin config fail";
  }
  if (is_add_record_)
    DB::Get("./")->CreateTable(DB::lib_name_, DB::model_name_);
  else if (is_recognize_)
    DB::Get("./");
}

void WarePlugin::ParseConfig() {
  is_recognize_ = config_->GetBoolValue("is_recognize");
  LOGE << "is_recognize: " << is_recognize_;
  is_add_record_ = config_->GetBoolValue("is_add_record");
  LOGE << "is_add_record: " << is_add_record_;
}

void SnapSmartMessage::ConvertSnapShotInfo(
    SnapshotInfoXStreamBaseDataPtr sp_snapshot,
    const xstream::BaseDataPtr face_feature,
    float x_ratio, float y_ratio,
    x3::Capture* capture_target) {
  auto ori_img = sp_snapshot->origin_image_frame;
  auto userdatas = sp_snapshot->userdata;
  capture_target->set_type_("face_snapshot");
  capture_target->set_timestamp_(ori_img->time_stamp);
//  uint8_t *src_data = reinterpret_cast<uint8_t *>(sp_snapshot->snap->Data());
//  int src_data_size = sp_snapshot->snap->DataSize();
//  int src_w = sp_snapshot->snap->Width();
//  int src_h = sp_snapshot->snap->Height();
//
//  //  fill crop img
//  auto capture_target_snap = capture_target->mutable_img_();
//  capture_target_snap->set_type_("NV12");
//  capture_target_snap->set_buf_(src_data, src_data_size);
//  capture_target_snap->set_height_(src_h);
//  capture_target_snap->set_width_(src_w);
//  LOGE << "capture_target_snap.size(): " << src_data_size;
//  LOGI << "snap userdata size:" << userdatas.size();

  /**
   * slot0: pose
   * slot1: lmk
   * slot2: face_bbox_list_after_filter
  **/
  int i = 0;
  for (auto &usrdata : userdatas) {
    if (usrdata->state_ != xstream::DataState::VALID) {
      LOGE << "state of " << usrdata->name_ << " is invalid";
      i++;
      continue;
    }
    if (i == 0) {
      auto face_pose = std::static_pointer_cast<
          xstream::XStreamData<hobot::vision::Pose3D>>(usrdata);
      auto capture_target_pose = capture_target->add_float_arrays_();
      capture_target_pose->set_type_("snap_pose");
      capture_target_pose->add_value_(face_pose->value.roll);
      capture_target_pose->add_value_(face_pose->value.pitch);
      capture_target_pose->add_value_(face_pose->value.yaw);
      capture_target_pose->add_value_(face_pose->value.score);
    } else if (i == 1) {
      auto face_lmks = std::static_pointer_cast<
          xstream::XStreamData<hobot::vision::Landmarks>>(usrdata);
      auto capture_target_lmks = capture_target->add_points_();
      capture_target_lmks->set_type_("snap_lmks");
      auto snap_lmks = sp_snapshot->PointsToSnap(face_lmks->value);
      for (auto &point : snap_lmks.values) {
        auto lmk = capture_target_lmks->add_points_();
        lmk->set_x_(point.x * x_ratio);
        lmk->set_y_(point.y * y_ratio);
        lmk->set_score_(point.score);
      }
    } else if (i == 2) {
      auto face_boxs = std::static_pointer_cast<
          xstream::XStreamData<hobot::vision::BBox>>(usrdata);
      auto capture_target_facebox = capture_target->add_boxes_();
      capture_target_facebox->set_type_("snap_face_box");
      auto pointtop = capture_target_facebox->mutable_top_left_();
      auto pointbottom = capture_target_facebox->mutable_bottom_right_();

      auto orig_points = Box2Points(face_boxs->value);
      auto snap_points = sp_snapshot->PointsToSnap(orig_points);
      auto snap_box = Points2Box(snap_points);
      snap_box.id = face_boxs->value.id;
      pointtop->set_x_(snap_box.x1 * x_ratio);
      pointtop->set_y_(snap_box.y1 * y_ratio);
      pointtop->set_score_(snap_box.score);
      pointbottom->set_x_(snap_box.x2 * x_ratio);
      pointbottom->set_y_(snap_box.y2 * y_ratio);
      pointbottom->set_score_(snap_box.score);
    }
    i++;
  }
  if (face_feature) {
    auto feature = std::static_pointer_cast<
        xstream::XStreamData<hobot::vision::Feature>>(face_feature);
    auto capture_target_feature = capture_target->add_float_arrays_();
    capture_target_feature->set_type_("face_feature");
    for (auto item : feature->value.values) {
      capture_target_feature->add_value_(item);
    }
  }
}

void SnapSmartMessage::ConvertRecognize(
    const xstream::BaseDataPtr face_feature,
    x3::DBResult* capture_dbresult) {
  if (face_feature) {
    HorizonVisionEncryptedFeature *vision_feature;
    HorizonRecogInfo recog_info;
    auto feature = std::static_pointer_cast<
        xstream::XStreamData<hobot::vision::Feature>>(face_feature);
    HorizonVisionAllocCharArray(&vision_feature);
    vision_feature->num = sizeof(float) * feature->value.values.size();
    vision_feature->values = static_cast<char *>(
        std::calloc(vision_feature->num, sizeof(char)));
    HobotAESEncrypt(reinterpret_cast<char *>(
                        feature->value.values.data()),
                    vision_feature->values,
                    AES_FEATURE, vision_feature->num);
    if (is_recognize_) {
      DB::Get()->Search(DB::lib_name_, DB::model_name_,
                        &vision_feature, 1, &recog_info, nullptr);
      if (recog_info.match) {
        LOGD << "find a matched person, id: " << recog_info.record_info.id
             << " similar: " << recog_info.similar;
        capture_dbresult->set_db_type_(DB::lib_name_);
        capture_dbresult->set_distance_(recog_info.distance);
        capture_dbresult->set_match_id_(recog_info.record_info.id);
        capture_dbresult->set_similarity_(recog_info.similar);
      }
    }
    if (is_add_record_) {
      char* const img_path = const_cast<char *>(image_name.c_str());
      DB::Get()->CreateRecordWithFeature(
          DB::lib_name_,
          std::to_string(frame_id), DB::model_name_, &img_path,
          &vision_feature, 1);
    }
    HorizonVisionFreeCharArray(vision_feature);
  } else {
    LOGE << "face_feature is null";
  }
}

void SnapSmartMessage::ConvertTargetInfo(
    const xstream::BaseDataVector *target_snaps,
    const xstream::BaseDataVector *target_features,
    float x_ratio, float y_ratio,
    x3::CaptureTarget* capture_targets) {
  xstream::BaseDataPtr face_feature = nullptr;
  auto snapshot_num = target_snaps->datas_.size();
  HOBOT_CHECK(snapshot_num > 0) << " one_target_snapshot_info has 0 snapshot";
  capture_targets->set_type_("snapshot");
  for (size_t i = 0; i < snapshot_num; ++i) {
    auto one_snap =
        dynamic_cast<XStreamSnapshotInfo *>(target_snaps->datas_[i].get());
    if (i == 0) capture_targets->set_track_id_(one_snap->value->track_id);
    face_feature = target_features->datas_[i];
    if (one_snap->state_ != xstream::DataState::VALID) {
      LOGE << "one_snap state is not Valid: "
           << static_cast<int>(one_snap->state_);
      return;
    }
    if (face_feature->state_ != xstream::DataState::VALID) {
      LOGE << "face_feature state is not Valid: "
           << static_cast<int>(face_feature->state_);
      return;
    }
    ConvertSnapShotInfo(one_snap->value, face_feature,
                        x_ratio, y_ratio,
                        capture_targets->add_captures_());
    ConvertRecognize(face_feature, capture_targets->add_db_results_());
  }
}

void SnapSmartMessage::ConvertSnapShotOutput(
    const xstream::BaseDataVector *targets_snapshots,
    const xstream::BaseDataVector *targets_face_features,
    float x_ratio, float y_ratio,
    x3::CaptureFrameMessage *capture_msg) {
  size_t num;
  xstream::BaseDataVector *target_features = nullptr;
  if (!targets_snapshots || !targets_face_features) {
    LOGD << "targets_snapshots or targets_face_features is null";
    return;
  }
  if ((num = targets_snapshots->datas_.size()) == 0) {
    LOGV << "Empty snapshots result";
    return;
  }
  if (targets_face_features &&
      targets_face_features->datas_.size() != num) {
    LOGF << "Face feature target size = "
         << targets_face_features->datas_.size()
         << " while snapshot target size = " << num;
  }
  for (size_t i = 0; i < num; ++i) {
    auto &target_snaps = targets_snapshots->datas_[i];
    auto p_target_snaps =
        dynamic_cast<xstream::BaseDataVector *>(target_snaps.get());
    // snap target with snapshot
    target_features =
        dynamic_cast<xstream::BaseDataVector *>(
            targets_face_features->datas_[i].get());
    ConvertTargetInfo(p_target_snaps, target_features,
                      x_ratio, y_ratio,
                      capture_msg->add_targets_());
  }
}

std::string SnapSmartMessage::Serialize(int ori_w, int ori_h, int dst_w,
                                          int dst_h) {
  HOBOT_CHECK(ori_w > 0 && ori_h > 0 && dst_w > 0 && dst_h > 0)
  << "Serialize param error";
  xstream::BaseDataVector *features = nullptr;
  xstream::BaseDataVector *snapinfo = nullptr;
  float x_ratio = 1.0 * dst_w / ori_w;
  float y_ratio = 1.0 * dst_h / ori_h;
  // serialize smart message using defined smart protobuf.
  std::string proto_str;
  x3::FrameMessage proto_frame_message;
  proto_frame_message.set_timestamp_(time_stamp);

  // add fps to output
  auto static_msg = proto_frame_message.mutable_statistics_msg_();
  auto fps_attrs = static_msg->add_attributes_();
  fps_attrs->set_type_("fps");
  fps_attrs->set_value_(frame_fps);
  fps_attrs->set_value_string_(std::to_string(frame_fps));

  for (const auto &output : smart_result->datas_) {
    LOGD << "output name: " << output->name_;
    if (output->name_ == "face_feature") {
      features = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << features->datas_.size();
    }
    if (output->name_ == "snap_list") {
      snapinfo = dynamic_cast<xstream::BaseDataVector *>(output.get());
      LOGD << output->name_ << " size: " << snapinfo->datas_.size();
    }
    if (features && snapinfo) {
      auto start = std::chrono::system_clock::now();
      ConvertSnapShotOutput(snapinfo, features, x_ratio, y_ratio,
                            proto_frame_message.mutable_capture_msg_());
      LOGD << "ConvertSnapShotOutput: " << std::chrono::duration_cast<
          std::chrono::milliseconds>(
              std::chrono::system_clock::now() - start).count() << " ms ";
      break;
    }
  }
  if (ap_mode_) {
    x3::MessagePack pack;
    pack.set_flow_(x3::MessagePack_Flow::MessagePack_Flow_CP2AP);
    pack.set_type_(x3::MessagePack_Type::MessagePack_Type_kXPlugin);
    if (channel_id == IMAGE_CHANNEL_FROM_AP) {  //  image is from ap
      pack.mutable_addition_()->mutable_frame_()->set_sequence_id_(frame_id);
    }
    pack.set_content_(proto_frame_message.SerializeAsString());
    pack.SerializeToString(&proto_str);
  } else {
    proto_frame_message.SerializeToString(&proto_str);
  }
  LOGD << "ware result serial success";
  return proto_str;
}

int WarePlugin::Init() {
  if (isinit_) return kHorizonVisionSuccess;
  isinit_ = true;
  LOGI << "WarePlugin INIT";
  RegisterMsg(TYPE_SMART_MESSAGE,
      std::bind(&WarePlugin::OnGetSmarterResult, this,
          std::placeholders::_1));
  return XPluginAsync::Init();
}

int WarePlugin::Start() {
  LOGI << "WarePlugin Start";
  return kHorizonVisionSuccess;
}

int WarePlugin::Stop() {
  LOGI << "WarePlugin Stop";
  return kHorizonVisionSuccess;
}

int WarePlugin::OnGetSmarterResult(const XProtoMessagePtr& msg) {
  if (std::dynamic_pointer_cast<SnapSmartMessage>(msg)) {
    return kHorizonVisionSuccess;
  }
  auto smart_msg = std::static_pointer_cast<
      smartplugin::CustomSmartMessage>(msg);
  HOBOT_CHECK(smart_msg);
  auto ware_msg = std::shared_ptr<SnapSmartMessage>(
      new SnapSmartMessage(*smart_msg.get()));
  ware_msg->SetAddRecordMode(is_add_record_);
  ware_msg->SetRecognizeMode(is_recognize_);
  PushMsg(ware_msg);
  return kHorizonVisionSuccess;
}

}  // namespace wareplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
