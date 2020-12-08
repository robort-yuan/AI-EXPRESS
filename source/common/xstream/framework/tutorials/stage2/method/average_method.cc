/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     Average Method
 * @author    zhe.sun
 * @version   0.0.0.1
 * @date      2020.10.31
 */

#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include "method/method.h"
#include "method/value.h"
#include "hobotlog/hobotlog.hpp"

namespace xstream {

int Average::Init(const std::string &config_file_path) {
  std::cout << "Average::Init" << std::endl;
  return 0;
}

void Average::Finalize() {
  std::cout << "Average::Finalize" << std::endl;
}

int Average::UpdateParameter(InputParamPtr ptr) {
  return 0;
}

InputParamPtr Average::GetParameter() const {
  return InputParamPtr();
}

std::string Average::GetVersion() const {
  return "";
}

MethodInfo Average::GetMethodInfo() {
  MethodInfo method_info = MethodInfo();
  method_info.is_thread_safe_ = false;  // 设置该Method非线程安全
  return method_info;
}

std::vector<std::vector<BaseDataPtr>> Average::DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) {
  std::cout << "Start Average::DoProcess..." << std::endl;
  std::cout << "Average Instance id: " << this << std::endl;
  std::cout << "Average Thread id: " << std::this_thread::get_id() << std::endl;

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
      // 计算输入数组的平均值
      auto in_datas = std::static_pointer_cast<BaseDataVector>(in_batch_i[j]);
      auto out_data = std::static_pointer_cast<FloatValue>(out_batch_i[j]);
      float sum = 0;
      int count = 0;
      for (auto &in_data : in_datas->datas_) {
        auto data = std::static_pointer_cast<FloatValue>(in_data);
        sum += data->value_;
        count++;
      }
      out_data->value_ = sum / count;
    }
  }
  return output;
}

}  // namespace xstream
