/**
 * Copyright (c) 2019, Horizon Robotics, Inc.
 * All rights reserved.
 * @Author:
 * @Mail: @horizon.ai
 */
#ifndef INCLUDE_SMARTPLUGIN_DISPLAYINFO_H_
#define INCLUDE_SMARTPLUGIN_DISPLAYINFO_H_

#include <stdint.h>
namespace horizon {
namespace vision {
class DisplayInfo {
 public:
  DisplayInfo(/* args */);
  ~DisplayInfo();

  static void computeXYPossition(const int dispaly_mode, const int channel_num,
                                 const int channel_id, int &x_offset,
                                 int &y_offset);

  static void computeResolution(const int dispaly_mode, const int channel_num,
                                const int channel_id, uint32_t &width,
                                uint32_t &height);

  static int computePymLayer(const int dispaly_mode, const int channel_num,
                             const int channel_id);
};

}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_VPSMODULE_H_
