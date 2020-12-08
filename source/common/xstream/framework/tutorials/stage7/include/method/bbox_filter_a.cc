/**
 * @copyright Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @file      bbox_filter_a.cc
 * @brief     BBoxFilterA class implementation
 * @author    Qingpeng Liu (qingpeng.liu@horizon.ai)
 * @version   0.0.0.1
 * @date      2020-01-03
 */

#include "method/bbox_filter_a.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include "hobotxstream/profiler.h"
#include "hobotxsdk/xstream_data.h"
#include "json/json.h"
#include "method/bbox.h"

namespace xstream {

int BBoxFilterA::Init(const std::string &config_file_path) {
  std::cout << "BBoxFilterA::Init " << config_file_path << std::endl;
  area_threshold_ = 100;
  std::cout << "BBoxFilterA::Init area_thres:" << area_threshold_ << std::endl;
  return 0;
}

int BBoxFilterA::UpdateParameter(InputParamPtr ptr) {
  return 0;
}

InputParamPtr BBoxFilterA::GetParameter() const {
  auto param = std::make_shared<BBoxFilterAParam>("");
  return param;
}

std::vector<std::vector<BaseDataPtr>> BBoxFilterA::DoProcess(
    const std::vector<std::vector<BaseDataPtr>> &input,
    const std::vector<InputParamPtr> &param) {

  std::cout << "BBoxFilterA::DoProcess begin " << input.size() << std::endl;
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
        auto bbox = std::static_pointer_cast<BBox>(in_rect);
        {
          std::cout << "filter out: "
                    << bbox->x1 << ","
                    << bbox->y1 << ","
                    << bbox->x2 << ","
                    << bbox->y2 << ", score: "
                    << bbox->score << std::endl;
        }
      }
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::cout << "BBoxFilterA::DoProcessing end " << std::endl;
  return output;
}

void BBoxFilterA::Finalize() {
  std::cout << "BBoxFilterA::Finalize" << std::endl;
}

std::string BBoxFilterA::GetVersion() const { return "test_only"; }

void BBoxFilterA::OnProfilerChanged(bool on) {}

}  // namespace xstream
