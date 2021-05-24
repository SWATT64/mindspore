/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINDSPORE_CCSRC_PS_CORE_SCHEDULER_NODE_H_
#define MINDSPORE_CCSRC_PS_CORE_SCHEDULER_NODE_H_

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "ps/core/cluster_config.h"
#include "ps/ps_context.h"
#include "ps/core/communicator/tcp_client.h"
#include "ps/core/communicator/tcp_server.h"
#include "ps/core/node_manager.h"
#include "ps/core/node.h"
#include "ps/core/communicator/request_process_result_code.h"
#include "ps/core/communicator/http_message_handler.h"
#include "ps/constants.h"
#include "ps/core/cluster_metadata.h"
#include "ps/core/communicator/http_server.h"

namespace mindspore {
namespace ps {
namespace core {
class SchedulerNode : public Node {
 public:
  SchedulerNode()
      : server_(nullptr),
        scheduler_thread_(nullptr),
        update_state_thread_(nullptr),
        restful_thread_(nullptr),
        http_server_(nullptr),
        client_thread_(nullptr),
        is_client_started_(false) {}
  ~SchedulerNode() override;

  typedef void (SchedulerNode::*ResponseHandler)(std::shared_ptr<TcpServer> server, std::shared_ptr<TcpConnection> conn,
                                                 std::shared_ptr<MessageMeta> meta, const void *data, size_t size);

  bool Start(const uint32_t &timeout = PSContext::instance()->cluster_config().cluster_available_timeout) override;
  bool Stop() override;
  bool Finish(const uint32_t &timeout = kTimeoutInSeconds) override;

 private:
  void Initialize();
  void InitCommandHandler();
  void CreateTcpServer();
  void StartUpdateClusterStateTimer();
  const std::shared_ptr<TcpClient> &GetOrCreateClient(const NodeInfo &node_info);

  void ProcessHeartbeat(std::shared_ptr<TcpServer> server, std::shared_ptr<TcpConnection> conn,
                        std::shared_ptr<MessageMeta> meta, const void *data, size_t size);
  void ProcessRegister(std::shared_ptr<TcpServer> server, std::shared_ptr<TcpConnection> conn,
                       std::shared_ptr<MessageMeta> meta, const void *data, size_t size);
  void ProcessFinish(std::shared_ptr<TcpServer> server, std::shared_ptr<TcpConnection> conn,
                     std::shared_ptr<MessageMeta> meta, const void *data, size_t size);
  void ProcessFetchMetadata(std::shared_ptr<TcpServer> server, std::shared_ptr<TcpConnection> conn,
                            std::shared_ptr<MessageMeta> meta, const void *data, size_t size);

  // After scheduler collects all registered message, it actively sends metadata to workers and servers.
  void SendMetadata(const std::shared_ptr<TcpClient> &client);
  // // After scheduler collects all finish message, it actively sends finish message to workers and servers.
  void SendFinish(const std::shared_ptr<TcpClient> &client);

  std::shared_ptr<TcpServer> server_;
  std::unique_ptr<std::thread> scheduler_thread_;
  std::unique_ptr<std::thread> update_state_thread_;
  std::unordered_map<NodeCommand, ResponseHandler> handlers_;

  NodeManager node_manager_;

  std::unique_ptr<std::thread> restful_thread_;
  std::shared_ptr<HttpServer> http_server_;

  std::unordered_map<std::string, std::shared_ptr<TcpClient>> connected_nodes_;

  std::unique_ptr<std::thread> client_thread_;
  std::atomic<bool> is_client_started_;
};
}  // namespace core
}  // namespace ps
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_PS_CORE_SCHEDULER_NODE_H_
