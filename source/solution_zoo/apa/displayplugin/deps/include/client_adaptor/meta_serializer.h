//
// Copyright 2017~2020 Horizon Robotics, Inc.
//

#ifndef INCLUDE_CLIENT_ADAPTOR_META_SERIALIZER_H_
#define INCLUDE_CLIENT_ADAPTOR_META_SERIALIZER_H_

#include "client_adaptor/type_def.h"

namespace horizon {
namespace auto_client_adaptor {
class MetaSerializer {
 public:
  int Serialize(const std::shared_ptr<frame_data_t> src,
                std::shared_ptr<std::vector<uint8_t>> *buffer);

 private:
  int GetVersion();
};
}  // namespace auto_client_adaptor
}  // namespace horizon

#endif  // INCLUDE_CLIENT_ADAPTOR_META_SERIALIZER_H_