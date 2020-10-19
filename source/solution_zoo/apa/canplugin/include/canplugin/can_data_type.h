/*
 * @Description: implement of can plugin
 * @Author: hangjun.yang@horizon.ai
 * @Date: 2020-09-10 19:00:00
 * @LastEditors: hangjun.yang@horizon.ai
 * @LastEditTime: 2020-09-10 19:00:00
 * @Copyright 2017~2020 Horizon Robotics, Inc.
 */

#ifndef INCLUDE_CANPLUGIN_CAN_DATA_TYPE_H_
#define  INCLUDE_CANPLUGIN_CAN_DATA_TYPE_H_

namespace horizon {
namespace vision {
namespace xproto {
namespace canplugin {

struct can_header {
  int64_t time_stamp;
  uint8_t counter;
  uint8_t frame_num;
  uint8_t channel;
  uint8_t type;
  uint8_t reserve[4];
};

struct can_frame {
  int32_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
  uint8_t can_dlc;  /* frame payload length in byte (0 .. CAN_MAX_DLEN) */
  uint8_t reserve[3];
  uint8_t data[8];
};

struct can_message {
  struct can_header header;
  struct can_frame frames[10];  // number of frames depend on real data
};

}  // namespace canplugin
}  // namespace xproto
}  // namespace vision
}  // namespace horizon
#endif  // INCLUDE_CANPLUGIN_CANPLUGIN_H_
