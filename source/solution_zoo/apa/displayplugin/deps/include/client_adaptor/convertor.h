//
// Copyright 2017~2020 Horizon Robotics, Inc.
//

#ifndef INCLUDE_CLIENT_ADAPTOR_CONVERTOR_H_
#define INCLUDE_CLIENT_ADAPTOR_CONVERTOR_H_

#include "client_adaptor/meta_serializer.h"

namespace horizon {
namespace auto_client_adaptor {

template <typename From_T, typename To_T>
class IConvertor {
 public:
  virtual int Convert(const std::shared_ptr<From_T> in,
                      std::shared_ptr<To_T>& out) = 0;
};

class ClientFrameConvertor : public IConvertor<frame_data_t, ClientFrame> {
 public:
  int Convert(const std::shared_ptr<frame_data_t> in,
              std::shared_ptr<ClientFrame>& out) override;
  private:
  MetaSerializer ser_;
};

}  // namespace auto_client_adaptor
}  // namespace horizon

#endif  // INCLUDE_CLIENT_ADAPTOR_CONVERTOR_H_