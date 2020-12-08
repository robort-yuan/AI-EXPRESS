/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     StandardDeviation Method
 * @author    zhe.sun
 * @version   0.0.0.1
 * @date      2020.10.31
 */

#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include "method/method.h"
#include "method/value.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int StandardDeviation::Init(const std::string &config_file_path) {
  std::cout << "StandardDeviation::Init" << std::endl;
  return 0;
}

void StandardDeviation::Finalize() {
  std::cout << "StandardDeviation::Finalize" << std::endl;
}

int StandardDeviation::UpdateParameter(InputParamPtr ptr) {
  return 0;
}

InputParamPtr StandardDeviation::GetParameter() const {
  return InputParamPtr();
}

std::string StandardDeviation::GetVersion() const {
  return "";
}

MethodInfo StandardDeviation::GetMethodInfo() {
  MethodInfo method_info = MethodInfo();
  method_info.is_thread_safe_ = true;  // 设置该Method线程安全
  return method_info;
}

std::vector<std::vector<BaseDataPtr>> StandardDeviation::DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) {
  std::cout << "Start StandardDeviation::DoProcess..." << std::endl;
  std::cout << "StandardDeviation Instance id: " << this << std::endl;
  std::cout << "StandardDeviation Thread id: " << std::this_thread::get_id()
            << std::endl;

  std::vector<std::vector<BaseDataPtr>> output;
  output.resize(input.size());  // batch size
  // one batch
  for (size_t i = 0; i < input.size(); ++i) {
    auto &in_batch_i = input[i];
    auto &out_batch_i = output[i];

    for (size_t j = 0; j < in_batch_i.size(); j++) {
      out_batch_i.push_back(std::make_shared<FloatValue>());
      if (in_batch_i[j]->state_ == DataState::INVALID) {
        std::cout << "input slot " << j << " is invalid" << std::endl;
        continue;
      }
      // 计算输入数组的标准差
      auto in_datas = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      auto out_data = std::static_pointer_cast<FloatValue>(out_batch_i[j]);
      float average = 0, standard_deviation = 0;
      int count = 0;
      // 平均值
      for (auto &in_data : in_datas->datas_) {
        auto data = std::static_pointer_cast<FloatValue>(in_data);
        average += data->value_;
        count++;
      }
      average /= count;
      for (auto &in_data : in_datas->datas_) {
        auto data = std::static_pointer_cast<FloatValue>(in_data);
        standard_deviation += (data->value_ - average) *
                              (data->value_ - average);
      }
      standard_deviation /= count;
      standard_deviation = sqrt(standard_deviation);
      out_data->value_ = standard_deviation;
    }
  }
  return output;
}

}  // namespace xstream
