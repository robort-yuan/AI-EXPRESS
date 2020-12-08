/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      alarm_method.cpp
 * @brief     Timeout_Alarm class implementation
 * @author    Ronghui Zhang (ronghui.zhang@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-10-26
 */

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <ctime>
#include "bbox.h"
#include "json/json.h"
#include "hobotxsdk/xstream_data.h"
#include "method/alarm_method.h"

#define MIN_VALUE 3
#define MAX_VALUE 10

namespace xstream {

int TimeoutAlarm::Init(const std::string &config_file_path) {
  std::cout << "TimeoutAlarm::Init " << config_file_path << std::endl;
  return 0;
}

std::vector<std::vector<BaseDataPtr>> TimeoutAlarm::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {
  std::cout << "TimeoutAlarm::DoProcess" << std::endl;
  unsigned int seed = time(0);
  int cost = rand_r(&seed) % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());  // batch size
  // one batch
  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];
    // one slot
    for (size_t j = 0; j < in_batch_i.size(); j++) {
      out_batch_i.push_back(std::make_shared<BaseDataVector>());
      if (in_batch_i[j]->state_ == DataState::INVALID) {
        std::cout << "input slot " << j << " is invalid" << std::endl;
        continue;
      }
      auto in_rects = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      auto out_rects = std::static_pointer_cast<BaseDataVector>(out_batch_i[j]);
      for (auto &in_rect : in_rects->datas_) {
          // passthrough data
        out_rects->datas_.push_back(in_rect);
      }
    }
  }
  std::cout << "sleep " << cost << " seconds" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(cost));
  return output;
}

int TimeoutAlarm::UpdateParameter(InputParamPtr ptr) {
  return 0;
}

InputParamPtr TimeoutAlarm::GetParameter() const {
  return InputParamPtr();
}

void TimeoutAlarm::Finalize() {
  std::cout << "TimeoutAlarm::Finalize" << std::endl;
}

std::string TimeoutAlarm::GetVersion() const {
  return "TimeoutAlarm test";
}

void TimeoutAlarm::OnProfilerChanged(bool on) {}
}  // namespace xstream
