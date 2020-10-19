//
// Copyright 2017~2020 Horizon Robotics, Inc.
//

#ifndef INCLUDE_CLIENT_ADAPTOR_NET_SENDER_H_
#define INCLUDE_CLIENT_ADAPTOR_NET_SENDER_H_

#include <string>

#include "client_adaptor/type_def.h"

namespace horizon {
namespace auto_client_adaptor {

class NetSender {
 public:
 NetSender() = default;
 ~NetSender();
  int Init(const std::string& endpoint);
  int SendClientFrame(const std::shared_ptr<ClientFrame> frame);

  private:
  void *context_;
  void *socket_;

  bool inited_{false};
  std::string endpoint_;
};

}  // namespace auto_client_adaptor
}  // namespace horizon
#endif  // INCLUDE_CLIENT_ADAPTOR_NET_SENDER_H_