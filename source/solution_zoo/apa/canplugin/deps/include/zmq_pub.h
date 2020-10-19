//
// Copyright 2020 Horizon Robotics.
//
#ifndef ZMQ_PUB_H_
#define ZMQ_PUB_H_
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

class Pub {
 public:
  Pub(std::string filepath) {
    if (filepath == "") {
      LOGE << "no zmq ipc file input";
    }
    file_path = filepath;
    if ((context = zmq_ctx_new()) == NULL) {
      LOGE << "pub context init failed";
    }
  }
  ~Pub() {
    zmq_close(puber);
    zmq_ctx_destroy(context);
  }
  int InitPub() {
    if (access(file_path.c_str(), F_OK) == -1) {
      if (creat(file_path.c_str(), 0755) < 0) {
        LOGE << "create file failed";
        return -1;
      }
    }
    std::string addr = "ipc://" + file_path;
    if ((puber = zmq_socket(context, ZMQ_PUB)) == NULL) {
      LOGE << " sub socket init failed";
      return -1;
    }
    int hwm = 10;
    int rc = zmq_setsockopt(puber, ZMQ_SNDHWM, &hwm, sizeof(hwm));
    if (rc < 0) {
      LOGE << "set sndhwm failed";
      return -1;
    }
    int linger = 1000;
    rc = zmq_setsockopt(puber, ZMQ_LINGER, &linger, sizeof(linger));
    if (rc < 0) {
      LOGE << "set linger failed";
      return -1;
    }
    int sndbuf = 16 * 1024;
    rc = zmq_setsockopt(puber, ZMQ_SNDBUF, &sndbuf, sizeof(sndbuf));
    if (rc < 0) {
      LOGE << "set sndbuf failed" << std::endl;
      return -1;
    }
    if (zmq_bind(puber, addr.c_str()) < 0) {
      LOGE << "pub bind failed: " << zmq_strerror(errno);
      return -1;
    }
    usleep(150000);
    pub_inite = true;
    return 0;
  }
  int IpcPub(uint8_t* data, int length) {
    if (!pub_inite) {
      if (InitPub() == -1) {
        LOGE << "pub init failed!";
        return 0;
      }
    }

    zmq_msg_t msg;
    int rc = 0;
    rc = zmq_msg_init_size(&msg, length);
    if (rc != 0) {
      return -1;
    }

    memcpy(zmq_msg_data(&msg), data, length);
    if (zmq_msg_send(&msg, puber, 0) < 0) {
      LOGE << "send message faild: " << stderr;
    }
    zmq_msg_close(&msg);
    return 0;
  }
  std::string file_path;
  bool pub_inite = false;
  void* context;
  void* puber;
};

#endif  // ZMQ_PUB_H_
