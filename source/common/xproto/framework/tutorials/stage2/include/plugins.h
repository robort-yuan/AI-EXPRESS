/*
 * Copyright (c) 2020 Horizon Robotics
 * @brief     definition of plugins
 * @author    zhe.sun
 * @date      2020.10.4
 */

#ifndef TUTORIALS_STAGE2_INCLUDE_PLUGINS_H_
#define TUTORIALS_STAGE2_INCLUDE_PLUGINS_H_

#include <iostream>
#include <chrono>
#include <string>
#include <memory>
#include <thread>
#include "number_message.h"
#include "xproto/plugin/xpluginasync.h"
#include "xproto/manager/msg_manager.h"
#include "hobotlog/hobotlog.hpp"

using horizon::vision::xproto::XPluginAsync;
using horizon::vision::xproto::XProtoMessagePtr;
using horizon::vision::xproto::XMsgQueue;

class NumberProducerPlugin : public XPluginAsync {
 public:
  std::string desc() const {
    return "NumberProducerPlugin";
  }
  int Init() {
    total_cnt_ = 5;
    prd_thread_ = nullptr;
    return XPluginAsync::Init();
  }
  int Start() {
    LOGI << "total_cnt=" << total_cnt_;
    LOGI << desc() << " Start";
    prd_thread_ = new std::thread([&] (){
      for (uint32_t i = 0; i < total_cnt_ && !prd_stop_; i++) {
        auto np_msg1 = std::make_shared<NumberProdMessage1>(1);
        // 向总线发送消息，若超出最大限制数量，则持续等待直到消息队列长度满足要求再发送
        PushMsg(np_msg1);
        LOGD << "PushMsg NumberProdMessage1 success";

        auto np_msg2 = std::make_shared<NumberProdMessage2>(1);
        // 向总线发送消息【可能失败】，若未超出最大限制数量，发送消息，返回成功；否则不再发送，返回失败
        int ret = TryPushMsg(np_msg2);
        if (ret == 0) {
          LOGD << "TryPushMsg NumberProdMessage2 success.";
        } else {
          LOGW << "TryPushMsg NumberProdMessage2 fail.";
        }
        std::this_thread::sleep_for(std::chrono::microseconds(50));
      }
    });
    return 0;
  }
  int Stop() {
    prd_stop_ = true;
    prd_thread_->join();
    if (prd_thread_) {
      delete prd_thread_;
    }
    LOGI << desc() << " Stop";
    return 0;
  }
  int DeInit() {
    return XPluginAsync::DeInit();
  }

 private:
  uint32_t total_cnt_;
  std::thread *prd_thread_;
  bool prd_stop_{false};
};

class SumConsumerPlugin : public XPluginAsync {
 public:
  int Init() override {
    sum_ = 0.f;
    RegisterMsg(TYPE_NUMBER_MESSAGE1, std::bind(&SumConsumerPlugin::Sum1,
                                               this, std::placeholders::_1));
    RegisterMsg(TYPE_NUMBER_MESSAGE2, std::bind(&SumConsumerPlugin::Sum2,
                                               this, std::placeholders::_1));
    return XPluginAsync::Init();
  }
  int Sum1(XProtoMessagePtr msg) {
    auto np_msg = std::static_pointer_cast<NumberProdMessage1>(msg);
    sum_ += np_msg->num_;
    LOGI << "Consume NumberProdMessage1, curr sum:" << sum_;

    std::this_thread::sleep_for(std::chrono::microseconds(500));
    return sum_;
  }
  int Sum2(XProtoMessagePtr msg) {
    auto np_msg = std::static_pointer_cast<NumberProdMessage2>(msg);
    sum_ += np_msg->num_;
    LOGI << "Consume NumberProdMessage2, curr sum:" << sum_;

    std::this_thread::sleep_for(std::chrono::microseconds(500));
    return sum_;
  }

  int Start() {
    LOGI << desc() << " Start";
    return 0;
  }
  int Stop() {
    LOGI << desc() << " Stop";
    return 0;
  }
  int DeInit() {
    return XPluginAsync::DeInit();
  }
  std::string desc() const {
    return "SumConsumerPlugin";
  }

 private:
  float sum_;
};

#endif  // TUTORIALS_STAGE2_INCLUDE_PLUGINS_H_
