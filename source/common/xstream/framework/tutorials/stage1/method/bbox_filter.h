/**
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     BBoxFilter Method
 * @author    zhe.sun
 * @version   0.0.0.1
 * @date      2020.10.30
 */

#ifndef XSTREAM_FRAMEWORK_TUTORIALS_STAGE1_METHOD_BBOX_FILTER_H_
#define XSTREAM_FRAMEWORK_TUTORIALS_STAGE1_METHOD_BBOX_FILTER_H_

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include "method/bbox.h"
#include "hobotxstream/method.h"

namespace xstream {

class BBoxFilterParam : public InputParam {
 public:
  explicit BBoxFilterParam(const std::string &module_name) :
           InputParam(module_name) {}
  std::string Format() override {
    return "";
  }
};

class BBoxFilter : public Method {
 private:
  float score_threshold_ = 0.5;

 public:
  int Init(const std::string &config_file_path) override {
    std::cout << "BBoxFilter Init" << std::endl;
    return 0;
  }

  std::vector<std::vector<BaseDataPtr>> DoProcess(
      const std::vector<std::vector<BaseDataPtr>> &input,
      const std::vector<xstream::InputParamPtr> &param) override {
      std::cout << "BBoxScoreFilter::DoProcess " << input.size() << std::endl;
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
        if (bbox->score > score_threshold_) {
          out_rects->datas_.push_back(in_rect);
        } else {
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
  return output;
  }

  void Finalize() override {
    std::cout << "BBoxFilter Finalize" << std::endl;
  }

  int UpdateParameter(InputParamPtr ptr) override {
    return 0;
  }

  InputParamPtr GetParameter() const override {
    auto param = std::make_shared<BBoxFilterParam>("");
    return param;
  }

  std::string GetVersion() const override {
    return "";
  }
  void OnProfilerChanged(bool on) override {}
};

}  // namespace xstream

#endif  // XSTREAM_FRAMEWORK_TUTORIALS_STAGE1_METHOD_BBOX_FILTER_H_
