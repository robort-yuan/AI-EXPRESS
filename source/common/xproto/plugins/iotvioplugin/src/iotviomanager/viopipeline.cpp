/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author: yong.wu
 * @Mail: yong.wu@horizon.ai
 * @Date: 2020-08-12
 * @Version: v1.0.0
 * @Brief: IOT VIO PipeLine for Horizon VIO System.
 */

#include "iotviomanager/viopipeline.h"
#include "iotviomanager/viopipeconfig.h"
#include "hobotlog/hobotlog.hpp"

namespace horizon {
namespace vision {
namespace xproto {
namespace vioplugin {

int VioPipeLine::Init() {
  int ret = -1;
  int cam_en = -1;
  int pipe_id = pipe_start_idx_;
  IotVioCfg vio_cfg = { 0 };
  IotVinParams *vin_params = nullptr;
  std::shared_ptr<VinModule> vin_module = nullptr;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VinModule>> vin_pair;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  ret = HbPipeConfig();
  HOBOT_CHECK(ret == 0) << "Vio pipe config failed";
  LOGI << "Enter viopipeline init, pipe_id: " << pipe_id
    << " pipe_sync_num: " << pipe_sync_num_;
  for (int index = 0; index < pipe_sync_num_; index++) {
    vio_cfg = vio_cfg_[pipe_id];
    cam_en = vio_cfg.cam_en;
    vin_params = &vin_params_[pipe_id];
    if (cam_en == 1) {
      /* Create Vinmodule */
      vin_module = std::make_shared<VinModule>(pipe_id, vio_cfg);
      vin_pair = make_pair(pipe_id, vin_module);
      vin_module_list_.push_back(vin_pair);
      ret = vin_module->VinInit();
      HOBOT_CHECK(ret == 0) << "vin module init failed!";
      ret = vin_module->GetVinConfig(vin_params);
      HOBOT_CHECK(ret == 0) << "vin module get vin config failed!";
      /* Create Vpsmodule */
      vps_module = std::make_shared<VpsModule>(pipe_id, vio_cfg, *vin_params);
      vps_pair = make_pair(pipe_id, vps_module);
      vps_module_list_.push_back(vps_pair);
      /* vps init */
      ret = vps_module->VpsInit();
      HOBOT_CHECK(ret == 0) << "vps module init failed!";
      /* bind vin to vps */
      ret = HbPipeSysBind(pipe_id);
      HOBOT_CHECK(ret == 0) << "pipe sys bind failed!";
      /* cam init */
      ret = vin_module->CamInit();
      HOBOT_CHECK(ret == 0) << "vin cam module init failed!";
      ret = vin_module->GetCamConfig(vin_params);
      HOBOT_CHECK(ret == 0) << "vin module get cam config failed!";
    } else {
      /* Create Vpsmodule */
      vps_module = std::make_shared<VpsModule>(pipe_id, vio_cfg);
      vps_pair = make_pair(pipe_id, vps_module);
      vps_module_list_.push_back(vps_pair);
      /* vps and fb init */
      ret = vps_module->FbInit();
      HOBOT_CHECK(ret == 0) << "fb module init failed!";
      ret = vps_module->VpsInit();
      HOBOT_CHECK(ret == 0) << "vps module init failed!";
    }
    pipe_id++;
  }

  return 0;
}

int VioPipeLine::DeInit() {
  int ret = -1;
  int cam_en = -1;
  int pipe_id = pipe_start_idx_;
  std::shared_ptr<VinModule> vin_module = nullptr;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VinModule>> vin_pair;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  for (int index = 0; index < pipe_sync_num_; index++) {
    cam_en = vio_cfg_[pipe_id].cam_en;
    vps_pair = vps_module_list_[index];
    vps_module = vps_pair.second;
    if (cam_en == 1) {
      vin_pair = vin_module_list_[index];
      vin_module = vin_pair.second;
      ret = vin_module->DeInit();
      HOBOT_CHECK(ret == 0) << "vin module deinit failed!";
    } else {
      ret = vps_module->FbDeInit();
      HOBOT_CHECK(ret == 0) << "fb module deinit failed!";
    }
    ret = vps_module->VpsDeInit();
    HOBOT_CHECK(ret == 0) << "vps module deinit failed!";
    pipe_id++;
  }

  return 0;
}

int VioPipeLine::Start() {
  int ret;
  int pipe_id, cam_en;
  std::shared_ptr<VinModule> vin_module = nullptr;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VinModule>> vin_pair;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  pipe_id = pipe_start_idx_;
  LOGI << "Enter viopipeline start, pipe_id: " << pipe_id
    << " pipe_sync_num: " << pipe_sync_num_;
  for (int index = 0; index < pipe_sync_num_; index++) {
    cam_en = vio_cfg_[pipe_id].cam_en;
    if (cam_en == 1) {
      vin_pair = vin_module_list_[index];
      vin_module = vin_pair.second;
      ret = vin_module->Start();
      if (ret) {
        LOGE << "vin module start failed, pipe_id: " << pipe_id;
        return ret;
      }
    }
    vps_pair = vps_module_list_[index];
    vps_module = vps_pair.second;
    ret = vps_module->VpsStart();
    if (ret) {
      LOGE << "vps module start failed, pipe_id: " << pipe_id;
      return ret;
    }
    pipe_id++;
  }

  return 0;
}

int VioPipeLine::Stop() {
  int ret;
  int pipe_id, cam_en;
  std::shared_ptr<VinModule> vin_module = nullptr;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VinModule>> vin_pair;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  LOGD << "Enter vio pipeline module stop";
  pipe_id = pipe_start_idx_;
  for (int index = 0; index < pipe_sync_num_; index++) {
    cam_en = vio_cfg_[pipe_id].cam_en;
    vps_pair = vps_module_list_[index];
    vps_module = vps_pair.second;
    LOGI << "viopipeline stop, cam_en: " << cam_en
      << " pipe_id: " << pipe_id;
    if (cam_en == 1) {
      vin_pair = vin_module_list_[index];
      vin_module = vin_pair.second;
      ret = vin_module->Stop();
      if (ret) {
        LOGE << "vin module stop failed!";
        return ret;
      }
    }
    ret = vps_module->VpsStop();
    if (ret) {
      LOGE << "vps module stop failed!";
      return ret;
    }
    pipe_id++;
  }

  return 0;
}


int VioPipeLine::GetInfo(uint32_t info_type, void *data) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(data);
  index = 0;
  vps_pair = vps_module_list_[index];
  pipe_id = vps_pair.first;
  vps_module = vps_pair.second;
  ret = vps_module->VpsGetInfo(info_type, data);
  if (ret) {
    LOGE << "vps module get info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::SetInfo(uint32_t info_type, void *data) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(data);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  vps_module = vps_pair.second;
  ret = vps_module->VpsSetInfo(info_type, data);
  if (ret) {
    LOGE << "vps module set info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::FreeInfo(uint32_t info_type, void *data) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(data);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  vps_module = vps_pair.second;
  ret = vps_module->VpsFreeInfo(info_type, data);
  if (ret) {
    LOGE << "vps module free info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

#if 0
int VioPipeLine::GetPymInfo(pym_buffer_t *pym_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(pym_info);
  index = 0;
  vps_pair = vps_module_list_[index];
  pipe_id = vps_pair.first;
  vps_module = vps_pair.second;
  ret = vps_module->VpsGetInfo(IOT_VIO_PYM_INFO, pym_info);
  if (ret) {
    LOGE << "get pym data failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::GetIpuInfo(hb_vio_buffer_t *ipu_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(ipu_info);
  index = 0;
  vps_pair = vps_module_list_[index];
  pipe_id = vps_pair.first;
  vps_module = vps_pair.second;
  ret = vps_module->VpsGetInfo(IOT_VIO_SRC_INFO, ipu_info);
  if (ret) {
    LOGE << "get ipu data failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::GetFbSrcInfo(hb_vio_buffer_t *fb_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(fb_info);
  pipe_id = pipe_start_idx_;
  index = 0;
  vps_pair = vps_module_list_[index];
  pipe_id = vps_pair.first;
  vps_module = vps_pair.second;
  ret = vps_module->VpsGetInfo(IOT_VIO_FEEDBACK_SRC_INFO, fb_info);
  if (ret) {
    LOGE << "get feedback src info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::SetFbPymProcess(hb_vio_buffer_t *src_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(src_info);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  vps_module = vps_pair.second;
  ret = vps_module->VpsSetInfo(IOT_VIO_FEEDBACK_PYM_PROCESS, src_info);
  if (ret) {
    LOGE << "set pym process info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::FreeIpuInfo(hb_vio_buffer_t *ipu_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(ipu_info);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  ret = vps_module->VpsFreeInfo(IOT_VIO_SRC_INFO, ipu_info);
  if (ret) {
    LOGE << "free ipu info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::FreePymInfo(pym_buffer_t *pym_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(pym_info);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  ret = vps_module->VpsFreeInfo(IOT_VIO_PYM_INFO, pym_info);
  if (ret) {
    LOGE << "free pym info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}

int VioPipeLine::FreeFeedbackSrcInfo(hb_vio_buffer_t *feed_info) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;

  CHECK_PARAMS_VALID(feed_info);
  index = 0;
  pipe_id = vps_pair.first;
  vps_pair = vps_module_list_[index];
  ret = vps_module->VpsFreeInfo(IOT_VIO_FEEDBACK_SRC_INFO, ipu_info);
  if (ret) {
    LOGE << "free ipu info failed, pipe_id: " << pipe_id;
    return ret;
  }

  return 0;
}
#endif

#if 0
int VioPipeLine::GetMultInfo(uint32_t info_type, std::vector<void*> &data) {
  int ret, index, pipe_id;
  std::shared_ptr<VpsModule> vps_module = nullptr;
  std::pair<int, std::shared_ptr<VpsModule>> vps_pair;
  int size = data.size();
  if (size > pipe_sync_num_) {
    LOGE << "vector size is error!";
      return -1;
  }

  for (index = 0; index < pipe_sync_num_; index++) {
    CHECK_PARAMS_VALID(data[index]);
    vps_pair = vps_module_list_[index];
    pipe_id = vps_pair.first;
    vps_module = vps_pair.second;
    ret = vps_module->VpsGetInfo(info_type, data[index]);
    if (ret) {
      LOGE << "vps module get info failed, pipe_id: " << pipe_id;
      return ret;
    }
  }
  return 0;
}
#endif

int VioPipeLine::GetMultInfo(uint32_t info_type, void *data) {
  /* TODO */

  return 0;
}

int VioPipeLine::SetMultInfo(uint32_t info_type, void *data) {
  /* TODO */

  return 0;
}

int VioPipeLine::FreeMultInfo(uint32_t info_type, void *data) {
  /* TODO */

  return 0;
}

int VioPipeLine::HbPipeSysBind(int pipe_id) {
  int ret;
  int vin_vps_mode;
  struct HB_SYS_MOD_S src_mod, dst_mod;

  vin_vps_mode = vio_cfg_[pipe_id].vin_vps_mode;
  src_mod.enModId = HB_ID_VIN;
  src_mod.s32DevId = pipe_id;
  if (vin_vps_mode == VIN_ONLINE_VPS_ONLINE ||
      vin_vps_mode == VIN_OFFLINE_VPS_ONLINE ||
      vin_vps_mode == VIN_SIF_ONLINE_DDR_ISP_ONLINE_VPS_ONLINE ||
      vin_vps_mode == VIN_SIF_OFFLINE_ISP_OFFLINE_VPS_ONLINE ||
      vin_vps_mode == VIN_FEEDBACK_ISP_ONLINE_VPS_ONLINE ||
      vin_vps_mode == VIN_SIF_VPS_ONLINE)
    src_mod.s32ChnId = 1;
  else
    src_mod.s32ChnId = 0;
  dst_mod.enModId = HB_ID_VPS;
  dst_mod.s32DevId = pipe_id;
  dst_mod.s32ChnId = 0;
  ret = HB_SYS_Bind(&src_mod, &dst_mod);
  if (ret != 0) {
    LOGE << "HB_SYS_Bind failed";
    return ret;
  }

  return 0;
}

int VioPipeLine::HbPipeConfig() {
  int pipe_id;
  IotVioCfg *vio_cfg = nullptr;

  pipe_sync_num_ = pipe_file_.size();
  HOBOT_CHECK(pipe_sync_num_ > 0);
  pipe_id = pipe_start_idx_;
  HOBOT_CHECK(pipe_id >= 0);
  for (int index = 0; index < pipe_sync_num_; index++) {
    /* parser vio pipeline config file */
    auto vio_json_cfg = std::make_shared<VioPipeConfig>(pipe_file_[index],
        pipe_id);
    if (!vio_json_cfg || !vio_json_cfg->LoadConfigFile()) {
      LOGE << "falied to load config file: " << pipe_file_[index];
      return -1;
    }
    vio_cfg = &vio_cfg_[pipe_id];
    vio_json_cfg->ParserConfig();
    vio_json_cfg->GetConfig(vio_cfg);
    pipe_id++;
  }

  return 0;
}

}  // namespace vioplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
