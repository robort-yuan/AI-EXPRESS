/* @Description: implementation of displayplugin
 * @Author: shiyu.fu@horizon.ai
 * @Date: 2020-09-09 18:45:37
 * @Last Modified by: shiyu.fu@horizon.ai
 * @Last Modified time: 2020-09-10 13:57:12
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 * */

#include "displayplugin/displayplugin.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "hobotlog/hobotlog.hpp"
#include "xproto/message/pluginflow/flowmsg.h"
#include "xproto/message/pluginflow/msg_registry.h"
#include "xproto/plugin/xpluginasync.h"

#include "hobotxsdk/xstream_sdk.h"
#include "horizon/vision/util.h"
#include "horizon/vision_type/vision_error.h"
#include "horizon/vision_type/vision_type.hpp"
#include "xproto_msgtype/vioplugin_data.h"
#include "xproto_msgtype/gdcplugin_data.h"
#include "xproto_msgtype/smartplugin_data.h"
#include "multismartplugin/smartplugin.h"
#include "utils/time_helper.h"

namespace horizon {
namespace vision {
namespace xproto {
namespace displayplugin {

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessage;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::basic_msgtype::VioMessage;
using horizon::vision::xproto::multivioplugin::MultiVioMessage;
using horizon::vision::xproto::basic_msgtype::IpmImageMessage;
using horizon::vision::xproto::basic_msgtype::SmartMessage;
using horizon::vision::xproto::multismartplugin::CustomSmartMessage;
using horizon::auto_client_adaptor::PerceptionObject;
using horizon::auto_client_adaptor::PerceptionObjectPtr;
using horizon::auto_client_adaptor::PerceptionObjectType;
using horizon::auto_client_adaptor::Segmentation;
using horizon::auto_client_adaptor::SegmentationPtr;
using horizon::auto_client_adaptor::ImageParameter;
using horizon::auto_client_adaptor::ImageParameterPtr;
using horizon::auto_client_adaptor::ImageFormat;
using horizon::auto_client_adaptor::CameraChannel;
using hobot::Timer;


int GetYUV(iot_venc_src_buf_t *frame_buf, VioMessage *vio_msg,
        int level) {
  // currently only support pyramid image
  LOGI << "x3 mediacodec: " << __FUNCTION__;
  if (!vio_msg || vio_msg->num_ == 0)
    return -1;
  auto pym_image = vio_msg->image_[0];
  auto height = pym_image->down_scale[level].height;
  auto width = pym_image->down_scale[level].width;
  auto stride = pym_image->down_scale[level].stride;
  auto y_vaddr = pym_image->down_scale[level].y_vaddr;
  auto y_paddr = pym_image->down_scale[level].y_paddr;
  auto c_vaddr = pym_image->down_scale[level].c_vaddr;
  auto c_paddr = pym_image->down_scale[level].c_paddr;
  HOBOT_CHECK(height) << "width = " << width << ", height = " << height;

  frame_buf->frame_info.width = width;
  frame_buf->frame_info.height = height;
  frame_buf->frame_info.stride = stride;
  frame_buf->frame_info.size = stride * height * 3 / 2;
  frame_buf->frame_info.vir_ptr[0] = reinterpret_cast<char *>(y_vaddr);
  frame_buf->frame_info.phy_ptr[0] = (uint32_t)y_paddr;
  frame_buf->frame_info.vir_ptr[1] = reinterpret_cast<char *>(c_vaddr);
  frame_buf->frame_info.phy_ptr[1] = (uint32_t)c_paddr;
  frame_buf->frame_info.pix_format = HB_PIXEL_FORMAT_NV12;
  return 0;
}

DisplayPlugin::DisplayPlugin(const std::string &config_file) {
  config_file_ = config_file;
  LOGI << "display config file:" << config_file_;
  std::ifstream infile(config_file_);
  infile >> cfg_jv_;

  smart_stop_flag_ = false;
  video_stop_flag_ = false;
  map_stop_ = false;

  Reset();
}

int DisplayPlugin::Init() {
  source_num_ =
    cfg_jv_.isMember("source_num") ? cfg_jv_["source_num"].asInt() : 1;
  port_ =
    cfg_jv_.isMember("port") ? cfg_jv_["port"].asString() : "tcp://*:7777";
  image_height_ =
    cfg_jv_.isMember("image_height") ? cfg_jv_["image_height"].asUInt() : 720;
  image_width_ =
    cfg_jv_.isMember("image_width") ? cfg_jv_["image_width"].asUInt() : 1280;
  frame_buf_depth_ =
    cfg_jv_.isMember("frame_buf_depth") ? cfg_jv_["frame_buf_depth"].asInt()
                                        : 3;
  is_cbr_ = cfg_jv_.isMember("is_cbr") ? cfg_jv_["is_cbr"].asInt() : 1;
  bitrate_ = cfg_jv_.isMember("bitrate") ? cfg_jv_["bitrate"].asInt() : 2000;
  jpeg_quality_ =
    cfg_jv_.isMember("jpeg_quality") ? cfg_jv_["jpeg_quality"].asUInt() : 100;
  if (jpeg_quality_ > 100) jpeg_quality_ = 100;
  use_vb_ = cfg_jv_.isMember("use_vb") ? cfg_jv_["use_vb"].asInt() : 0;
  layer_ = cfg_jv_.isMember("layer") ? cfg_jv_["layer"].asInt() : 0;

  image_seg_type_ =
  cfg_jv_.isMember("image_seg_type") ? cfg_jv_["image_seg_type"].asInt() :
        6;
  ipm_seg_type_ =
  cfg_jv_.isMember("ipm_seg_type") ? cfg_jv_["ipm_seg_type"].asInt() :
        10;

  adaptor_ = std::make_shared<ClientAdaptor>(port_);
  jpeg_encode_thread_.CreatThread(1);
  data_send_thread_.CreatThread(1);

  RegisterMsg(TYPE_IMAGE_MESSAGE,
              std::bind(&DisplayPlugin::OnVioMessage, this,
                        std::placeholders::_1));
  RegisterMsg(TYPE_MULTI_IMAGE_MESSAGE,
              std::bind(&DisplayPlugin::OnVioMessage, this,
                        std::placeholders::_1));
  RegisterMsg(TYPE_IPM_MESSAGE,
              std::bind(&DisplayPlugin::OnIpmMessage, this,
                        std::placeholders::_1));
  RegisterMsg(TYPE_SMART_MESSAGE,
              std::bind(&DisplayPlugin::OnSmartMessage, this,
                        std::placeholders::_1));
  return XPluginAsync::Init();
}

int DisplayPlugin::Start() {
  /* 1. media codec init */
  /* 1.1 get media codec manager and module init */
  MediaCodecManager &manager = MediaCodecManager::Get();
  auto rv = manager.ModuleInit();
  HOBOT_CHECK(rv == 0);
  /* 1.2 get media codec venc chn */
  chn_ = manager.GetEncodeChn();
  /* 1.3 media codec venc chn init */
  rv = manager.EncodeChnInit(chn_, PT_JPEG, image_width_, image_height_,
          frame_buf_depth_, HB_PIXEL_FORMAT_NV12, is_cbr_, bitrate_);
  HOBOT_CHECK(rv == 0);
  /* 1.4 set media codec venc jpg chn qfactor params */
  rv = manager.SetUserQfactorParams(chn_, jpeg_quality_);
  HOBOT_CHECK(rv == 0);
  /* 1.5 set media codec venc jpg chn qfactor params */
  rv = manager.EncodeChnStart(chn_);
  HOBOT_CHECK(rv == 0);
  /* 1.6 alloc media codec vb buffer init */
  if (use_vb_) {
    int vb_num = frame_buf_depth_;
    int pic_size = image_width_ * image_height_ * 3 / 2;  // nv12 format
    int vb_cache_enable = 1;
    rv = manager.VbBufInit(chn_, image_width_, image_height_, image_width_,
        pic_size, vb_num, vb_cache_enable);
    HOBOT_CHECK(rv == 0);
  }
  LOGI << "media codec manager & module inited";

  if (!worker_) {
    worker_ = std::make_shared<std::thread>(
        std::bind(&DisplayPlugin::MapSmartProc, this));
  }

  LOGW << "DisplayPlugin Start";
  return 0;
}

int DisplayPlugin::Stop() {
  {
    std::lock_guard<std::mutex> smart_lock(smart_mutex_);
    smart_stop_flag_ = true;
  }
  {
    std::lock_guard<std::mutex> video_lock(video_mutex_);
    video_stop_flag_ = true;
  }
  {
    if (worker_ && worker_->joinable()) {
      map_stop_ = true;
      map_smart_condition_.notify_one();
      worker_->join();
      worker_ = nullptr;
      LOGI << "display plugin stop worker";
    }
  }

  /* 3. media codec deinit */
  /* 3.1 media codec chn stop */
  MediaCodecManager &manager = MediaCodecManager::Get();
  manager.EncodeChnStop(chn_);
  /* 3.2 media codec chn deinit */
  manager.EncodeChnDeInit(chn_);
  /* 3.3 media codec vb buf deinit */
  if (use_vb_) {
    manager.VbBufDeInit(chn_);
  }
  /* 3.4 media codec module deinit */
  manager.ModuleDeInit();
  LOGI << "media codec deinited";
  LOGW << "DisplayPlugin Stop";
  return 0;
}

int DisplayPlugin::Reset() {
  std::unique_lock<std::mutex> lock(map_mutex_);

  for (auto itr = pym_img_buffer_.begin();
       itr != pym_img_buffer_.end(); ++itr) {
    auto &frames = itr->second;
    while (!frames.empty()) {
      frames.pop();
    }
  }

  for (auto itr = smart_msg_buffer_.begin();
       itr != smart_msg_buffer_.end(); ++itr) {
    auto &smarts = itr->second;
    while (!smarts.empty()) {
      smarts.pop();
    }
  }
  pym_img_buffer_.clear();
  smart_msg_buffer_.clear();
  return 0;
}

int DisplayPlugin::OnVioMessage(XProtoMessagePtr msg) {
  if (msg->type_ == TYPE_IMAGE_MESSAGE) {
    // todo
  } else if (msg->type_ == TYPE_MULTI_IMAGE_MESSAGE) {
    LOGI << "received multi image message";
    auto multi_vio_msg = std::static_pointer_cast<MultiVioMessage>(msg);
    for (size_t i = 0; i < multi_vio_msg->multi_vio_img_.size(); ++i) {
      auto one_chn_img = multi_vio_msg->multi_vio_img_[i];
      jpeg_encode_thread_.PostTask(
        std::bind(&DisplayPlugin::EncodeJpeg, this, one_chn_img));
    }
  } else {
    LOGE << "received unknown message type: " << msg->type_;
    return -1;
  }
  return 0;
}

int DisplayPlugin::OnIpmMessage(XProtoMessagePtr msg) {
  // handle ipm message
  return 0;
}

int DisplayPlugin::OnSmartMessage(XProtoMessagePtr msg) {
  {
    std::lock_guard<std::mutex> smart_lock(smart_mutex_);
    if (smart_stop_flag_) {
      LOGD << "Aleardy stop, display plugin FeedSmart return";
      return -1;
    }
  }
  auto smart_msg = std::static_pointer_cast<SmartMessage>(msg);
  if (smart_msg) {
    int ch_id = smart_msg->channel_id;
    LOGI << "display plugin got one smart result, ch_id:" << ch_id;
    {
      std::lock_guard<std::mutex> smart_lock(map_mutex_);
      smart_msg_buffer_[ch_id].push(smart_msg);
    }
    map_smart_condition_.notify_one();
  }
  return 0;
}

void DisplayPlugin::EncodeJpeg(std::shared_ptr<ImageVioMessage> vio_msg) {
  bool check_cost_time = false;
  auto check_cost_time_str = getenv("check_cost_time");
  if (check_cost_time_str && !strcmp(check_cost_time_str, "ON")) {
    check_cost_time = true;
  }

  if (vio_msg->type_ == TYPE_IMAGE_MESSAGE) {
    auto valid_frame = vio_msg->image_[0];
    auto channel_id = valid_frame->channel_id;
    auto time_stamp = valid_frame->time_stamp;
    auto frame_id = valid_frame->frame_id;

    int rv;
    cv::Mat yuv_img;
    std::vector<uchar> img_buf;
    iot_venc_src_buf_t *frame_buf = nullptr;
    iot_venc_src_buf_t src_buf = { 0 };
    {
      std::lock_guard<std::mutex> video_lock(video_mutex_);
      if (video_stop_flag_) {
        LOGD << "Aleardy stop, DisplayPLugin Feedvideo return";
        return;
      }
    }
    LOGI << "DisplayPlugin EncodedJpeg";
    LOGI << "encoding pym image frame of channel: " << channel_id;
    // TODO(shiyu.fu): what if using other layers
    auto img_height = valid_frame->down_scale[0].height;
    auto img_width = valid_frame->down_scale[0].width;

    /* 2. start encode yuv to jpeg */
    /* 2.1 get media codec vb buf for store src yuv data */
    MediaCodecManager &manager = MediaCodecManager::Get();
    frame_buf = &src_buf;
    memset(frame_buf, 0x00, sizeof(iot_venc_src_buf_t));
    frame_buf->frame_info.pts = vio_msg->time_stamp_;
    /* 2.2 get src yuv data */
    rv = GetYUV(frame_buf, vio_msg.get(), layer_);
    if (rv != 0) {
      LOGF << "Convertor::GetYUV failed";
    }
    /* 2.3. encode yuv data to jpg */
    auto ts0 = Timer::current_time_stamp();
    rv = manager.EncodeYuvToJpg(chn_, frame_buf, img_buf);
    auto ts1 = Timer::current_time_stamp();
    if (check_cost_time) {
      LOGI << "******encodec cost: " << ts1 - ts0 << "ms";
    }
    if (rv == 0) {
      LOGI << "encode jpg success";
      // pack into client adaptor dtype
      ImageDataPtr img_data = std::make_shared<std::vector<uint8_t>>();
      img_data->assign(img_buf.begin(), img_buf.end());

      ImageParameterPtr img_param = std::make_shared<ImageParameter>();
      img_param->height = img_height;
      img_param->width = img_width;
      img_param->cam_id = channel_id;
      img_param->frame_id = frame_id;
      img_param->timestamp = time_stamp;
      img_param->format = ImageFormat::JPEG;

      CameraChannelPtr channel = std::make_shared<CameraChannel>();
      channel->param = img_param;
      channel->data = img_data;

      // push to buffer
      {
        std::lock_guard<std::mutex> lock(map_mutex_);
        pym_img_buffer_[channel_id].push(channel);
        if (pym_img_buffer_[channel_id].size() > cache_size_)
          LOGW << "pym_img_buffer channel: " << channel_id
               << " is full, size: " << pym_img_buffer_[channel_id].size()
               << " maybe the encode thread is slowly";
      }
      map_smart_condition_.notify_one();
    } else {
      LOGE << "X3 media codec jpeg encode failed!";
    }
  } else if (vio_msg->type_ == TYPE_IPM_MESSAGE) {
    // ipm image
  } else {
    LOGE << "received known image frame type: " << vio_msg->type_;
  }
}

int DisplayPlugin::SendClientMessage(std::shared_ptr<frame_data_t> one_frame) {
  int ret = adaptor_->SendToClient(one_frame);
  if (ret != 0) {
    LOGE << "failed to send data to matrix client, code: " << ret;
    return -1;
  }

  return 0;
}

void DisplayPlugin::MapSmartProc() {
  bool check_cost_time = false;
  auto check_cost_time_str = getenv("check_cost_time");
  if (check_cost_time_str && !strcmp(check_cost_time_str, "ON")) {
    check_cost_time = true;
  }
  while (!map_stop_) {
    std::unique_lock<std::mutex> lock(map_mutex_);
    map_smart_condition_.wait(lock);
    if (map_stop_) {
      break;
    }
    // all channel must have data
    if (smart_msg_buffer_.size() == 4 && pym_img_buffer_.size() == 4) {
      LOGD << "image buffer size: channel 0: " << pym_img_buffer_[0].size()
           << ", channel 1: " << pym_img_buffer_[1].size()
           << ", channel 2: " << pym_img_buffer_[2].size()
           << ", channel 3: " << pym_img_buffer_[3].size();
      LOGD << "smart buffer size: channel 0: " << smart_msg_buffer_[0].size()
           << ", channel 1: " << smart_msg_buffer_[1].size()
           << ", channel 2: " << smart_msg_buffer_[2].size()
           << ", channel 3: " << smart_msg_buffer_[3].size();
      // all channel should be ready for one frame
      bool ready_to_send = true;
      for (auto iter = smart_msg_buffer_.begin();
           iter != smart_msg_buffer_.end(); ++iter) {
        int ch_id = iter->first;
        if (iter->second.empty()) {
          ready_to_send = false;
          LOGI << "channel " << ch_id << " smart buffer empty";
          break;
        }
        if (pym_img_buffer_.count(ch_id) == 0 ||
            pym_img_buffer_[ch_id].empty()) {
          ready_to_send = false;
          LOGI << "channel " << ch_id << " image buffer not exist or empty";
          break;
        }
        auto &smarts = iter->second;
        auto &images = pym_img_buffer_[ch_id];
        auto msg = smarts.top();
        auto image = images.top();
        if (msg->frame_id != static_cast<uint32_t>(image->param->frame_id)) {
          ready_to_send = false;
          LOGI << "channel " << ch_id << " frame ids do not match: "
               << "msg frame_id: " << msg->frame_id << ", image frame_id: "
               << image->param->frame_id;
          if (channel_err_cnt_.find(ch_id) == channel_err_cnt_.end()) {
            channel_err_cnt_[ch_id] = 1;
          } else {
            channel_err_cnt_[ch_id] = channel_err_cnt_[ch_id] + 1;
          }
          if (channel_err_cnt_[ch_id] > 100) {
            LOGI << "channel " << ch_id << "err cnt: "
                 << channel_err_cnt_[ch_id] << ", PopBuffer()";
            PopBuffer(ch_id);
            channel_err_cnt_[ch_id] = 0;
          }
          break;
        }
      }
      // pack data into frame_data_t
      if (ready_to_send) {
        static int frame_data_frame_id = 0;
        std::shared_ptr<frame_data_t> one_frame =
          std::make_shared<frame_data_t>();
        one_frame->frame_id = frame_data_frame_id;
        LOGI << "smart_msg_buffer.size(): " << smart_msg_buffer_.size();
        for (auto smart_itr = smart_msg_buffer_.begin();
            smart_itr != smart_msg_buffer_.end(); ++smart_itr) {
          int ch_id = smart_itr->first;
          if (channel_err_cnt_.find(ch_id) != channel_err_cnt_.end()) {
            channel_err_cnt_[ch_id] = 0;
          }
          if (smart_itr->second.empty()) {
            LOGI << "empty continue";
            continue;
          }
          if (pym_img_buffer_.count(ch_id) == 0 ||
              pym_img_buffer_[ch_id].empty()) {
            LOGI << "image buffer not exist or empty";
            continue;
          }
          auto &smarts = smart_itr->second;
          auto &images = pym_img_buffer_[ch_id];

          auto msg = smarts.top();
          auto frame = images.top();
          if (msg->frame_id == static_cast<uint32_t>(frame->param->frame_id)) {
            auto ts0_assign = Timer::current_time_stamp();
            AssignSmartMessage(msg, frame);
            auto ts1_assign = Timer::current_time_stamp();
            if (check_cost_time) {
              LOGI << "******AssignSmartMessage cost: " <<
              ts1_assign - ts0_assign << "ms";
            }
            one_frame->channels.push_back(frame);
            LOGI << "push channel " << ch_id << " to one_frame";
            smarts.pop();
            images.pop();
          } else {
            LOGW << "time stamp not equal";
          }
        }
        // for each channel assign the same id
        for (const auto item : one_frame->channels) {
          item->param->frame_id = frame_data_frame_id;
        }

        data_send_thread_.PostTask(
        std::bind(&DisplayPlugin::SendClientMessage, this, one_frame));
        frame_data_frame_id++;
      }
    }
  }
}

void DisplayPlugin::AssignSmartMessage(SmartMessagePtr msg,
                                     CameraChannelPtr frame) {
  HOBOT_CHECK(msg->channel_id == frame->param->cam_id) << "channel id mismatch";
  LOGI << "assigning smart message to frame";
  auto get_perception_box_type =
    [](const std::string name) -> PerceptionObjectType {
    if (name == "cycle_box") {
      return PerceptionObjectType::Cyclist;
    } else if (name == "person_box") {
      return PerceptionObjectType::Person;
    } else if (name == "vehicle_box") {
      return PerceptionObjectType::Vehicle;
    } else if (name == "rear_box") {
      return PerceptionObjectType::VehicleRear;
    } else if (name == "parkinglock_box") {
      return PerceptionObjectType::ParkingLock;
    } else {
      return PerceptionObjectType::Unknown;
    }
  };

  auto smart_rets = msg->ConvertData();
  ApaSmartResult *apa_smart_ret = static_cast<ApaSmartResult *>(smart_rets);
  for (auto iter = apa_smart_ret->bbox_list_.begin();
       iter != apa_smart_ret->bbox_list_.end();
       ++iter) {
    auto chn_id = iter->first;
    LOGI << "assign smart ret chn_id: " << chn_id;
    for (const auto box : iter->second) {
      PerceptionObjectPtr perception = std::make_shared<PerceptionObject>();
      perception->type = get_perception_box_type(box.category_name);
      perception->id = box.id;
      perception->rect =
        horizon::auto_client_adaptor::Rect{box.x1, box.y1, box.x2, box.y2};
      perception->score = box.score;
      frame->perception_results.push_back(perception);
    }
  }
  for (auto iter = apa_smart_ret->segmentation_list_.begin();
       iter != apa_smart_ret->segmentation_list_.end();
       ++iter) {
    auto chn_id = iter->first;
    LOGI << "assign segmentation ret chn_id: " << chn_id;
    for (const auto seg : iter->second) {
      SegmentationPtr segment = std::make_shared<Segmentation>();
      // size needs to have the same ratio as 720 * 1280
      // actual segmentation output size: 176 * 320
      segment->height = 180;
      segment->width = 320;
      segment->score = seg.score;
      segment->semantics = image_seg_type_;

      segment->values = std::make_shared<std::vector<uint8_t>>();
      segment->values->assign(seg.values.begin(), seg.values.end());
      segment->values->resize(180 * 320, 0);

      frame->segment.push_back(segment);
    }
  }

  delete apa_smart_ret;
}

void DisplayPlugin::PopBuffer(int channel_id) {
  auto &smarts = smart_msg_buffer_[channel_id];
  auto &images = pym_img_buffer_[channel_id];
  auto top_smart = smarts.top();
  auto top_image = images.top();
  while (top_smart->frame_id !=
         static_cast<uint32_t>(top_image->param->frame_id)) {
    LOGI << "top_smart: " << top_smart->frame_id
         << " top_image: " << top_image->param->frame_id;
    if (top_smart->frame_id <
        static_cast<uint32_t>(top_image->param->frame_id)) {
      smarts.pop();
      LOGW << "pop smarts";
    }
    if (!smarts.empty() && !images.empty()) {
      top_smart = smarts.top();
      top_image = images.top();
    }
  }
}

}  // namespace displayplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
