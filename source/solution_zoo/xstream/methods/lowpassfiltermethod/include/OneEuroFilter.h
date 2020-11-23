 /*
 * Copyright (c) 2020 Horizon Robotics. All rights reserved.
 * @brief     OneEuroFilter
 * @author    ronghui.zhang
 * @email     ronghui.zhang@horizon.ai
 * @version   0.0.0.1
 * @date      2020.11.02
 */

#ifndef INCLUDE_ONEENROFILTER_H_
#define INCLUDE_ONEENROFILTER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "LowPassFilterMethod/data_type.h"
#include "hobotxstream/method.h"
#include "LowPassFilter.h"

namespace xstream {
class OneEuroFilter {
  double freq;
  double mincutoff;
  double beta_;
  double dcutoff;
  LowPassFilter *x;
  LowPassFilter *dx;
  TimeStamp lasttime;

double alpha(double cutoff) {
  double te = 1.0 / freq;
  double tau = 1.0 / (2*M_PI*cutoff);
  return 1.0 / (1.0 + tau/te);
}

void setFrequency(double f) {
  if (f <= 0) {
    throw std::range_error("freq should be >0");
  }
  freq = f;
}

void setMinCutoff(double mc) {
  if (mc <= 0) {
    throw std::range_error("mincutoff should be >0");
  }
  mincutoff = mc;
}

void setBeta(double b) {
  beta_ = b;
}

void setDerivateCutoff(double dc) {
  if (dc <= 0) {
    throw std::range_error("dcutoff should be >0");
  }
  dcutoff = dc;
}

 public:
  OneEuroFilter(double freq,
  double mincutoff = 0.3, double beta_ = 1.0, double dcutoff = 0.3) {
  setFrequency(freq);
  setMinCutoff(mincutoff);
  setBeta(beta_);
  setDerivateCutoff(dcutoff);
  x = new LowPassFilter(alpha(mincutoff));
  dx = new LowPassFilter(alpha(dcutoff));
  lasttime = UndefinedTime;
}

OneEuroFilter(const OneEuroFilter& filter) {
  freq = filter.freq;
  mincutoff = filter.mincutoff;
  beta_ = filter.beta_;
  dcutoff = filter.dcutoff;
  x = filter.x;
  dx = filter.dx;
  lasttime = filter.lasttime;
}

double filter(double value, TimeStamp timestamp = UndefinedTime) {
  //  update the sampling frequency based on timestamps
  if (lasttime != UndefinedTime && timestamp != UndefinedTime
     && timestamp != lasttime) {
    freq = 1.0 / (timestamp-lasttime);
  }
  lasttime = timestamp;
  //  estimate the current variation per second
  double dvalue = x->hasLastRawValue() ?
  (value - x->lastRawValue()) * freq : 0.0;  //  FIXME: 0.0 or value?
  //  double dvalue = last_value ? (value - last_value) * freq : 0.0;
  double edvalue = dx->filterWithAlpha(dvalue, alpha(dcutoff));
  // use it to update the cutoff frequency
  double cutoff = mincutoff + beta_ * fabs(edvalue);
  // filter the given value
  return x->filterWithAlpha(value, alpha(cutoff));
}

~OneEuroFilter(void) {
  delete x;
  delete dx;
}
};

}  // namespace xstream

#endif  // INCLUDE_MERGEMETHOD_MERGEMETHOD_H_
