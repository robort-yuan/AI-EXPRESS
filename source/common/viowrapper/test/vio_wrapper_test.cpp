/**
 * * Copyright (c) 2019 Horizon Robotics. All rights reserved.
 * @File: vio_wrapper_test.cpp
 * @Brief: vio wrapper unit test
 * @Author: xudong.du
 * @Email: xudong.du@horizon.ai
 * @Date: 2020-05-14
 * @Last Modified by: xudong.du
 * @Last Modified time: 2020-05-14
 */

#include "./vio_wrapper.h"

#include <gtest/gtest.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "hobotlog/hobotlog.hpp"
#ifdef X3
#include "./vio_wrapper_global.h"
#endif

extern char* sensor_type;
extern int test_num;
extern int dump_en;

/*
TEST(VIO_WRAPPER_TEST, GetMultiImage) {
  std::string vio_cfg_file = "./config/96board/vio_onsemi_dual.json";
  std::string cam_cfg_file = "./config/hb_x2dev.json";

  mult_img_info_t data;
  int ret = 0;
  HbVioDualCamera hd_vio(vio_cfg_file, cam_cfg_file);
  ret = hd_vio.Init();
  ASSERT_TRUE(ret == 0);    // NOLINT
  ret = hd_vio.GetMultiImage(&data);
  ASSERT_TRUE(ret == 0);    // NOLINT
  hd_vio.Free(&data);
}
*/

TEST(VIO_WRAPPER_TEST, GetSingleImage_1080) {
  std::string vio_cfg_file;
  std::string cam_cfg_file;
#ifdef X2
  vio_cfg_file = "./config/96board/vio_onsemi0230.json.96board";
  cam_cfg_file = "/etc/cam/hb_96board.json";
#endif
#ifdef X3
  cam_cfg_file = "./config/x3dev/hb_camera_x3.json";
#ifdef X3_X2_VIO
  vio_cfg_file = "./config/x3dev/hb_vio_x3_1080.json";
#endif
#ifdef X3_IOT_VIO
  if (sensor_type == "imx327") {
    vio_cfg_file = "./config/x3dev/iot_vio_x3_imx327.json";
  } else if (sensor_type == "os8a10") {
    vio_cfg_file = "./config/x3dev/iot_vio_x3_os8a10.json";
  } else if (sensor_type == "s5kgm") {
    vio_cfg_file = "./config/x3dev/iot_vio_x3_s5kgm1sp_2160p.json";
  } else {
    LOGE << "not support sensor type";
    return;
  }
#endif
#endif
  int ret = 0;
#ifdef X2
  img_info_t data;
  HbVioMonoCamera single_camera(vio_cfg_file, cam_cfg_file);
  ret = single_camera.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  ret = single_camera.GetImage(&data);
  ASSERT_TRUE(ret == 0);  // NOLINT
  vio_debug::print_info(data);
  vio_debug::dump_pym_nv12(&data, "single_camera");
  single_camera.Free(&data);
#endif
#ifdef X3
  int step = 0;
  HbVioMonoCameraGlobal single_camera(vio_cfg_file, cam_cfg_file);
  ret = single_camera.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  LOGD << "test_num: " << test_num;
  while (step < test_num) {
    LOGD << "step: " << step;
    std::shared_ptr<PymImageFrame> data = single_camera.GetImage();
    if (dump_en) {
      vio_debug::dump_pym_nv12(data, "single_camera");
    }
    single_camera.Free(data);
    step++;
  }
#endif
}

TEST(VIO_WRAPPER_TEST, GetFbImage_1080) {
  std::string img_list = "./data/image.list";
#ifdef X2
  std::string vio_cfg_file = "./config/vio_onsemi0230_fb.json";
#endif
#ifdef X3
   // must be same image resolution in image.list
  std::string vio_cfg_file = "./config/x3dev/iot_vio_x3_1080_fb.json";
#endif
  int ret = 0;
  std::ifstream ifs(img_list);
  ASSERT_TRUE(ifs.is_open());
#ifdef X2
  HbVioFbWrapper fb_handle(vio_cfg_file);
#endif
#ifdef X3
  HbVioFbWrapperGlobal fb_handle(vio_cfg_file);
#endif
  ret = fb_handle.Init();
  ASSERT_TRUE(ret == 0);  // NOLINT
  std::string input_image;
  std::string gt_data;
  while (getline(ifs, gt_data)) {
    std::cout << gt_data << std::endl;
    std::istringstream gt(gt_data);
    gt >> input_image;
    int pos=input_image.find_last_of('/');
    std::string file_name_bak(input_image.substr(pos+1));
    std::string file_name = file_name_bak.substr(0, file_name_bak.rfind("."));
    uint32_t effective_w, effective_h;
#ifdef X2
    img_info_t data;
    ret = fb_handle.GetImgInfo(input_image, &data, &effective_w, &effective_h);
    ASSERT_TRUE(ret == 0);  // NOLINT
    vio_debug::print_info(data);
    vio_debug::dump_pym_nv12(&data, file_name);
    fb_handle.FreeImgInfo(&data);
#endif
#ifdef X3
    std::shared_ptr<PymImageFrame> data;
    data = fb_handle.GetImgInfo(input_image, &effective_w, &effective_h);
    vio_debug::dump_pym_nv12(data, "fb_1080");
    fb_handle.FreeImgInfo(data);
#endif
  }
}
