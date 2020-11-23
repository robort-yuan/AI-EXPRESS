/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#ifndef INCLUDE_VENCMODULE_H_
#define INCLUDE_VENCMODULE_H_

#include <mutex>
#include <thread>
#include <vector>
#include <memory>

#include "hb_comm_venc.h"
#include "hobotxsdk/xstream_data.h"
#include "visualplugin/horizonserver_api.h"

namespace horizon {
namespace vision {

struct VencModuleInfo {
  uint32_t width;
  uint32_t height;
  uint32_t type;
  uint32_t bits;
};

struct VencBuffer {
  char *mmz_vaddr = nullptr;
  uint64_t mmz_paddr;
  uint32_t mmz_size;
  uint32_t mmz_flag = 0;
  std::mutex mmz_mtx;
};

struct VencData {
  uint32_t channel;
  uint32_t width;
  uint32_t height;

  char *y_virtual_addr;
  char *uv_virtual_addr;
};

struct VencConfig {
  uint32_t input_num;
};

class VencModule {
 public:
  VencModule();
  ~VencModule();
  int Init(uint32_t chn_id, const VencModuleInfo *module_info,
           const VencConfig &smart_venc_cfg);
  int Start();
  int Input(void *data, const xstream::OutputDataPtr &xstream_out);
  int Output(void **data);
  int OutputBufferFree(void *data);
  int Stop();
  int DeInit();
  int Process();

 private:
  int VencChnAttrInit(VENC_CHN_ATTR_S *pVencChnAttr, PAYLOAD_TYPE_E p_enType,
                      int p_Width, int p_Height, PIXEL_FORMAT_E pixFmt);
  static std::once_flag flag_;
  uint32_t chn_id_;
  uint32_t timeout_;

  VencModuleInfo venc_info_;
  VencBuffer buffers_;

  FILE *outfile_;
  int pipe_fd_;

  bool process_running_;
  std::shared_ptr<std::thread> process_thread_;

  VencConfig server_cfg_;
};

}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VencModule_H_
