//
// Copyright 2017~2020 Horizon Robotics, Inc.
//

#ifndef INCLUDE_CLIENT_ADAPTOR_CLIENT_ADAPTOR_H_
#define INCLUDE_CLIENT_ADAPTOR_CLIENT_ADAPTOR_H_

#include "client_adaptor/type_def.h"
#include "client_adaptor/convertor.h"
#include "client_adaptor/net_sender.h"

namespace horizon {
namespace auto_client_adaptor {

class __attribute__((visibility ("default"))) ClientAdaptor {
 public:
  explicit ClientAdaptor(const std::string& endpoint);
  ~ClientAdaptor() = default;
  int SendToClient(
      const std::shared_ptr<horizon::auto_client_adaptor::frame_data_t> frame);

 private:
  std::shared_ptr<ClientFrameConvertor> client_cvt_;
  std::shared_ptr<NetSender> net_sender_;
};

}  // namespace auto_client_adaptor
}  // namespace horizon

#endif  // INCLUDE_CLIENT_ADAPTOR_CLIENT_ADAPTOR_H_