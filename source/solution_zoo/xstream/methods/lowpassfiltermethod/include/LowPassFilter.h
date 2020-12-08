 /*
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     LowPassFilter
 * @author    ronghui.zhang
 * @email     ronghui.zhang@horizon.ai
 * @version   0.0.0.1
 * @date      2020.11.02
 */

#ifndef INCLUDE_LOWPASSFILTER_H_
#define INCLUDE_LOWPASSFILTER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <ctime>
#include "LowPassFilterMethod/data_type.h"
#include "hobotxstream/method.h"


namespace xstream {
// -----------------------------------------------------------------
typedef double TimeStamp;  // in seconds

static const TimeStamp UndefinedTime = -1.0;
// -----------------------------------------------------------------
class LowPassFilter {
  double y, a, s;
  bool initialized;

  void setAlpha(double alpha) {
    if (alpha <= 0.0 || alpha > 1.0) {
      throw std::range_error("alpha should be in (0.0., 1.0]");
    }
    a = alpha;
  }

 public:
  explicit LowPassFilter(double alpha, double initval = 0.0) {
    y = s = initval;
    setAlpha(alpha);
    initialized = false;
  }

  double filter(double value) {
    double result;
    if (initialized) {
      result = a * value + (1.0 - a) * s;
    } else {
      result = value;
      initialized = true;
    }
    y = value;
    s = result;
    return result;
  }

  double filterWithAlpha(double value, double alpha) {
    setAlpha(alpha);
    return filter(value);
  }

  bool hasLastRawValue(void) {
    return initialized;
  }

  double lastRawValue(void) {
    return y;
  }
};
}  // namespace xstream

#endif  // INCLUDE_LOWPASSFILTER_H_
