//
// Copyright 2020 Horizon Robotics.
//
#ifndef ZMQ_SUB_H_
#define ZMQ_SUB_H_

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include "hobotlog/hobotlog.hpp"

class Sub {
 public:
  explicit Sub(std::string filepath) {
    if (filepath == "") {
      LOGE << "no zmq ipc file input";
    }
    file_path = filepath;
    if ((context = zmq_ctx_new()) == NULL) {
      LOGE << "sub context init failed";
    }
  }
  ~Sub() {
    zmq_close(suber);
    zmq_ctx_destroy(context);
  }
  void SetSub(bool inite) {
    sub_inite = inite;
  }
  int InitSub() {
    if (access(file_path.c_str(), F_OK) == -1) {
      if (creat(file_path.c_str(), 0755) < 0) {
        LOGE << "create file failed";
        return -1;
      }
    }
    std::string addr = "ipc://" + file_path;
    if ((suber = zmq_socket(context, ZMQ_SUB)) == NULL) {
      LOGE << " sub socket init failed";
      zmq_ctx_destroy(context);
      return -1;
    }
    int hwm = 5;
    int rc = zmq_setsockopt(suber, ZMQ_RCVHWM, &hwm, sizeof(hwm));
    if (rc < 0) {
      LOGE << "set recvhwm failed";
      return -1;
    }
    int rcvbuf = 1024 * 1024;
    rc = zmq_setsockopt(suber, ZMQ_RCVBUF, &rcvbuf, sizeof(rcvbuf));
    if (rc < 0) {
      LOGE << "set recv buf failed";
      return -1;
    }
    zmq_setsockopt(suber, ZMQ_SUBSCRIBE, "", 0);
    if (zmq_connect(suber, addr.c_str()) < 0) {
      LOGE << "sub connect failed: " << zmq_strerror(errno);
      return -1;
    }
    LOGI << "connect to: " << addr;
    sub_inite = true;
    return 0;
  }

  int IpcSub(uint8_t* data) {
    if (!sub_inite) {
      if (InitSub() == -1) {
        LOGE << "init sub failed!";
        return -1;
      }
    }
    int iRcvTimeout = 5000;
    if (zmq_setsockopt(suber, ZMQ_RCVTIMEO, &iRcvTimeout, sizeof(iRcvTimeout)) <
        0) {
      LOGE << "set time out failed";
      return 0;
    }

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int ret;
    ret = zmq_msg_recv(&msg, suber, 0);
    if (ret < 0) {
#if 0
      if (errno == EAGAIN) {
        LOGE << "No message";
      }
      LOGE << "error = " << zmq_strerror(errno);
#endif
      return -1;
    }
    memcpy(data, zmq_msg_data(&msg), ret);
    zmq_msg_close(&msg);
    return ret;
  }
  std::string file_path;
  bool sub_inite = false;
  void* context;
  void* suber;
  void* puber;
};

#endif  // ZMQ_SUB_H_
