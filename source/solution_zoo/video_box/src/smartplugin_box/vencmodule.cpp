/**
 * Copyright (c) 2020, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#include "vencmodule.h"

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

#include "hb_venc.h"
#include "hb_vio_interface.h"
#include "hb_vp_api.h"
#include "hobotlog/hobotlog.hpp"
#include "visualplugin/horizonserver_api.h"

namespace horizon {
namespace vision {
std::once_flag VencModule::flag_;

VencModule::VencModule() : chn_id_(-1), timeout_(50), pipe_fd_(-1) {
  std::call_once(flag_, []() {
    int ret = HB_VENC_Module_Init();
    if (ret != 0) {
      LOGF << "HB_VENC_Module_Init Failed. ret = " << ret;
    }
  });
}

VencModule::~VencModule() { HB_VENC_Module_Uninit(); }

int VencModule::VencChnAttrInit(VENC_CHN_ATTR_S *pVencChnAttr,
                                PAYLOAD_TYPE_E p_enType, int p_Width,
                                int p_Height, PIXEL_FORMAT_E pixFmt) {
  int streambuf = 2 * 1024 * 1024;

  memset(pVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
  pVencChnAttr->stVencAttr.enType = p_enType;

  pVencChnAttr->stVencAttr.u32PicWidth = p_Width;
  pVencChnAttr->stVencAttr.u32PicHeight = p_Height;

  pVencChnAttr->stVencAttr.enMirrorFlip = DIRECTION_NONE;
  pVencChnAttr->stVencAttr.enRotation = CODEC_ROTATION_0;
  pVencChnAttr->stVencAttr.stCropCfg.bEnable = HB_FALSE;
  if (p_Width * p_Height > 2688 * 1522) {
    streambuf = 2 * 1024 * 1024;
  } else if (p_Width * p_Height > 1920 * 1080) {
    streambuf = 1024 * 1024;
  } else if (p_Width * p_Height > 1280 * 720) {
    streambuf = 512 * 1024;
  } else {
    streambuf = 256 * 1024;
  }
  if (p_enType == PT_JPEG || p_enType == PT_MJPEG) {
    pVencChnAttr->stVencAttr.enPixelFormat = pixFmt;
    pVencChnAttr->stVencAttr.u32BitStreamBufferCount = 1;
    pVencChnAttr->stVencAttr.u32FrameBufferCount = 2;
    pVencChnAttr->stVencAttr.bExternalFreamBuffer = HB_TRUE;
    pVencChnAttr->stVencAttr.stAttrJpeg.dcf_enable = HB_FALSE;
    pVencChnAttr->stVencAttr.stAttrJpeg.quality_factor = 0;
    pVencChnAttr->stVencAttr.stAttrJpeg.restart_interval = 0;
    pVencChnAttr->stVencAttr.u32BitStreamBufSize = streambuf;
  } else {
    pVencChnAttr->stVencAttr.enPixelFormat = pixFmt;
    pVencChnAttr->stVencAttr.u32BitStreamBufferCount = 3;
    pVencChnAttr->stVencAttr.u32FrameBufferCount = 3;
    pVencChnAttr->stVencAttr.bExternalFreamBuffer = HB_TRUE;
    pVencChnAttr->stVencAttr.u32BitStreamBufSize = streambuf;
  }

  if (p_enType == PT_H265) {
    pVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
    pVencChnAttr->stRcAttr.stH265Vbr.bQpMapEnable = HB_TRUE;
    pVencChnAttr->stRcAttr.stH265Vbr.u32IntraQp = 20;
    pVencChnAttr->stRcAttr.stH265Vbr.u32IntraPeriod = 50;
    pVencChnAttr->stRcAttr.stH265Vbr.u32FrameRate = 25;
  }
  if (p_enType == PT_H264) {
    pVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
    pVencChnAttr->stRcAttr.stH264Vbr.bQpMapEnable = HB_TRUE;
    pVencChnAttr->stRcAttr.stH264Vbr.u32IntraQp = 20;
    pVencChnAttr->stRcAttr.stH264Vbr.u32IntraPeriod = 50;
    pVencChnAttr->stRcAttr.stH264Vbr.u32FrameRate = 25;
    pVencChnAttr->stVencAttr.stAttrH264.h264_profile = HB_H264_PROFILE_MP;
    pVencChnAttr->stVencAttr.stAttrH264.h264_level = HB_H264_LEVEL1;
  }

  pVencChnAttr->stGopAttr.u32GopPresetIdx = 3;
  pVencChnAttr->stGopAttr.s32DecodingRefreshType = 2;

  return 0;
}

int VencModule::Init(uint32_t chn_id, const VencModuleInfo *module_info,
                     const VencConfig &smart_venc_cfg) {
  VENC_CHN_ATTR_S vencChnAttr;

  VENC_RC_ATTR_S *pstRcParam;

  int width = module_info->width;
  int height = module_info->height;
  PAYLOAD_TYPE_E ptype = PT_H264;
  VencChnAttrInit(&vencChnAttr, ptype, width, height, HB_PIXEL_FORMAT_NV12);

  int s32Ret = HB_VENC_CreateChn(chn_id, &vencChnAttr);
  if (s32Ret != 0) {
    printf("HB_VENC_CreateChn %d failed, %d.\n", chn_id, s32Ret);
    return -1;
  }

  if (ptype == PT_H264) {
    pstRcParam = &(vencChnAttr.stRcAttr);
    vencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    s32Ret = HB_VENC_GetRcParam(chn_id, pstRcParam);
    if (s32Ret != 0) {
      printf("HB_VENC_GetRcParam failed.\n");
      return -1;
    }

    printf(" vencChnAttr.stRcAttr.enRcMode = %d mmmmmmmmmmmmmmmmmm   \n",
           vencChnAttr.stRcAttr.enRcMode);
    printf(" u32VbvBufferSize = %d mmmmmmmmmmmmmmmmmm   \n",
           vencChnAttr.stRcAttr.stH264Cbr.u32VbvBufferSize);

    pstRcParam->stH264Cbr.u32BitRate = module_info->bits;
    pstRcParam->stH264Cbr.u32FrameRate = 25;
    pstRcParam->stH264Cbr.u32IntraPeriod = 50;
    pstRcParam->stH264Cbr.u32VbvBufferSize = 3000;
  } else if (ptype == PT_H265) {
    pstRcParam = &(vencChnAttr.stRcAttr);
    vencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
    s32Ret = HB_VENC_GetRcParam(chn_id, pstRcParam);
    if (s32Ret != 0) {
      printf("HB_VENC_GetRcParam failed.\n");
      return -1;
    }
    printf(" m_VencChnAttr.stRcAttr.enRcMode = %d mmmmmmmmmmmmmmmmmm   \n",
           vencChnAttr.stRcAttr.enRcMode);
    printf(" u32VbvBufferSize = %d mmmmmmmmmmmmmmmmmm   \n",
           vencChnAttr.stRcAttr.stH265Cbr.u32VbvBufferSize);

    pstRcParam->stH265Cbr.u32BitRate = module_info->bits;
    pstRcParam->stH265Cbr.u32FrameRate = 25;
    pstRcParam->stH265Cbr.u32IntraPeriod = 50;
    pstRcParam->stH265Cbr.u32VbvBufferSize = 3000;
  } else if (ptype == PT_MJPEG) {
    pstRcParam = &(vencChnAttr.stRcAttr);
    vencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
    s32Ret = HB_VENC_GetRcParam(chn_id, pstRcParam);
    if (s32Ret != 0) {
      printf("HB_VENC_GetRcParam failed.\n");
      return -1;
    }
  }

  s32Ret = HB_VENC_SetChnAttr(chn_id, &vencChnAttr);  // config
  if (s32Ret != 0) {
    printf("HB_VENC_SetChnAttr failed\n");
    return -1;
  }

  chn_id_ = chn_id;
  venc_info_.width = module_info->width;
  venc_info_.height = module_info->height;
  venc_info_.type = module_info->type;
  venc_info_.bits = module_info->bits;

  s32Ret = HB_VP_Init();
  if (s32Ret != 0) {
    printf("vp_init fail s32Ret = %d !\n", s32Ret);
  }

  buffers_.mmz_size = venc_info_.width * venc_info_.height * 3 / 2;
  s32Ret = HB_SYS_Alloc(&buffers_.mmz_paddr,
                        reinterpret_cast<void **>(&buffers_.mmz_vaddr),
                        buffers_.mmz_size);
  if (s32Ret == 0) {
    printf("mmzAlloc paddr = 0x%lx, vaddr = 0x%p \n", buffers_.mmz_paddr,
            buffers_.mmz_vaddr);
  }

  // char stream_name[100] = {0};
  // sprintf(stream_name, "%s%d%s", "./video_box/output_stream_", chn_id_,
  //         ".h264");
  // outfile_ = fopen(stream_name, "wb");

  server_cfg_ = smart_venc_cfg;
  return s32Ret;
}

int VencModule::Start() {
  int ret = 0;
  VENC_RECV_PIC_PARAM_S pstRecvParam;
  pstRecvParam.s32RecvPicNum = 0;  // unchangable
  ret = HB_VENC_StartRecvFrame(chn_id_, &pstRecvParam);
  if (ret != 0) {
    LOGE << "HB_VENC_StartRecvStream Failed. ret = " << ret;
    return ret;
  }

  process_running_ = true;
  process_thread_ = std::make_shared<std::thread>(&VencModule::Process, this);

  return ret;
}

int VencModule::Process() {
  const char *fifo_name = nullptr;

  if (0 == chn_id_) {
    fifo_name = "/tmp/h264_fifo";
  } else {
    fifo_name = "/tmp/h264_fifo1";
  }

  if (access(fifo_name, F_OK) == -1) {
    int res = mkfifo(fifo_name, 0777);
    if (res != 0) {
      LOGE << "mkdir fifo failed!!!!";
      return -1;
    }
  }

  // 会阻塞在这里，直到读取进程打开该fifo
  pipe_fd_ = open(fifo_name, O_WRONLY);
  if (pipe_fd_ == -1) {
    LOGE << "open fifo fail";
    return -1;
  }

  VIDEO_STREAM_S pstStream;
  while (process_running_) {
    memset(&pstStream, 0, sizeof(VIDEO_STREAM_S));
    int ret = HB_VENC_GetStream(chn_id_, &pstStream, timeout_);
    if (ret < 0) {
      printf("HB_VENC_GetStream error!!!\n");
    } else {
      // fwrite(pstStream.pstPack.vir_ptr, 1, pstStream.pstPack.size, outfile_);
      write(pipe_fd_, (unsigned char *)pstStream.pstPack.vir_ptr,
            pstStream.pstPack.size);
      LOGD << "Venc chn " << chn_id_ << " get stream pack size "
           << pstStream.pstPack.size;
      HB_VENC_ReleaseStream(chn_id_, &pstStream);
    }
  }

  return 0;
}

int VencModule::Input(void *data, const xstream::OutputDataPtr &xstream_out) {
  int ret = 0;
  VencData *venc_data = static_cast<VencData *>(data);

  // 拷贝数据到Buffer相应位置
  auto dst_start_vaddr =
      buffers_.mmz_vaddr + venc_data->width * (venc_data->channel % 2) +
      venc_info_.width * venc_data->height * (venc_data->channel / 2);
  auto src_start_vaddr = venc_data->y_virtual_addr;
  for (uint32_t j = 0; j < venc_data->height; j++) {
    memcpy(dst_start_vaddr, src_start_vaddr, venc_data->width);
    src_start_vaddr += venc_data->width;
    dst_start_vaddr += venc_info_.width;
  }

  dst_start_vaddr =
      buffers_.mmz_vaddr + venc_info_.width * venc_info_.height +
      venc_data->width * (venc_data->channel % 2) +
      venc_info_.width * venc_data->height * (venc_data->channel / 2) / 2;
  src_start_vaddr = venc_data->uv_virtual_addr;
  for (uint32_t j = 0; j < venc_data->height / 2; j++) {
    memcpy(dst_start_vaddr, src_start_vaddr, venc_data->width);
    src_start_vaddr += venc_data->width;
    dst_start_vaddr += venc_info_.width;
  }

  buffers_.mmz_flag |= 1 << venc_data->channel;

  if (!(buffers_.mmz_flag ^ ((1 << server_cfg_.input_num) - 1))) {
    // 送给编码器编码
    VIDEO_FRAME_S pstFrame;
    memset(&pstFrame, 0, sizeof(VIDEO_FRAME_S));

    pstFrame.stVFrame.width = venc_info_.width;
    pstFrame.stVFrame.height = venc_info_.height;
    pstFrame.stVFrame.size = buffers_.mmz_size;
    pstFrame.stVFrame.pix_format = HB_PIXEL_FORMAT_NV12;

    pstFrame.stVFrame.phy_ptr[0] = buffers_.mmz_paddr;
    pstFrame.stVFrame.phy_ptr[1] =
        buffers_.mmz_paddr + venc_info_.width * venc_info_.height;

    pstFrame.stVFrame.vir_ptr[0] = buffers_.mmz_vaddr;
    pstFrame.stVFrame.vir_ptr[1] =
        buffers_.mmz_vaddr + venc_info_.width * venc_info_.height;
    pstFrame.stVFrame.pts = 0;

    ret = HB_VENC_SendFrame(chn_id_, &pstFrame, timeout_);
    if (ret != 0) {
      LOGE << "HB_VENC_SendStream Failed. ret = " << ret;
    }

    // fwrite(buffers_.mmz_vaddr, 1, buffers_.mmz_size, outfile_);

    buffers_.mmz_flag = 0;
  }

  return ret;
}

int VencModule::Stop() {
  int ret = 0;

  process_running_ = false;
  if (pipe_fd_ > 0)
    process_thread_->join();

  LOGE << "VENC Stop id: " << chn_id_;
  ret = HB_VENC_StopRecvFrame(chn_id_);
  if (ret != 0) {
    LOGE << "HB_VENC_StopRecvStream Failed. ret = " << ret;
    return ret;
  }

  return ret;
}

int VencModule::DeInit() {
  int ret = 0;
  ret = HB_VENC_DestroyChn(chn_id_);
  if (ret != 0) {
    LOGE << "HB_VENC_DestroyChn Failed. ret = " << ret;
    return ret;
  }

  return ret;
}

}  // namespace vision
}  // namespace horizon
