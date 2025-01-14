/**
 * Copyright 2022 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CCSRC_INCLUDE_BACKEND_DATA_QUEUE_DATA_QUEUE_MGR_H
#define MINDSPORE_CCSRC_INCLUDE_BACKEND_DATA_QUEUE_DATA_QUEUE_MGR_H

#include <unistd.h>
#include <iostream>
#include <functional>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "include/backend/visible.h"
#include "include/backend/data_queue/data_queue.h"
#ifndef BUILD_LITE
#include "ir/anf.h"
#endif

namespace mindspore {
namespace device {
constexpr unsigned int MAX_WAIT_TIME_IN_SEC = 60;

class BlockingQueue;

// channel_name, dynamic_shape, capacity, addr, shape
using DataQueueCreator =
  std::function<std::shared_ptr<DataQueue>(const std::string &, bool, size_t, void *, const std::vector<size_t> &)>;

class Semaphore {
 public:
  explicit Semaphore(int count = 0) : count_(count) {}

  inline void Signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  inline bool Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ == 0) {
      if (cv_.wait_for(lock, std::chrono::seconds(MAX_WAIT_TIME_IN_SEC)) == std::cv_status::timeout) {
        return false;
      }
    }
    --count_;
    return true;
  }

 private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

class BACKEND_EXPORT DataQueueMgr {
 public:
  DataQueueMgr() : init_(false), closed_(false), open_by_dataset_(0) {}

  virtual ~DataQueueMgr() = default;

  static DataQueueMgr &GetInstance() noexcept;
  void RegisterDataQueueCreator(const std::string &device_name, DataQueueCreator &&creator);
  std::shared_ptr<DataQueue> CreateDataQueue(const std::string &device_name, const std::string &channel_name,
                                             bool dynamic_shape, size_t capacity, void *addr,
                                             const std::vector<size_t> &shape);

  DataQueueStatus Create(const std::string &channel_name, void *addr, const std::vector<size_t> &shape,
                         const size_t &capacity);

  // call for Push thread
  DataQueueStatus Open(const std::string &channel_name, std::function<void(void *, int32_t)> func);

  // call for Front/Pop thread
  DataQueueStatus Open(const std::string &channel_name) const;
  DataQueueStatus Push(const std::string &channel_name, const std::vector<DataQueueItem> &data,
                       unsigned int timeout_in_sec);
  DataQueueStatus Front(const std::string &channel_name, std::vector<DataQueueItem> *data);
  DataQueueStatus Pop(const std::string &channel_name);
  DataQueueStatus Clear(const std::string &channel_name);

  DataQueueStatus OpenDynamicBufQueue(const std::string &channel_name, const std::function<void(void *, int32_t)> func);
  DataQueueStatus CreateDynamicBufQueue(const std::string &channel_name, const size_t &capacity);
  DataQueueStatus OpenDynamicBufQueue(const std::string &channel_name);
  std::shared_ptr<DataQueue> GetDataQueue(const std::string &channel_name) const;
  DataQueueStatus SetThreadDevice(const std::string &device_name);
  std::shared_ptr<void> AllocHostMem(const std::string &device_name, size_t size);

  void Close(const std::string &channel_name) const noexcept;

  bool IsInit() const;

  bool IsClosed() const;

  bool Destroy();

  // call for Release GPU Resources
  bool CloseNotify();

  // call for dataset send thread
  void CloseConfirm();

  size_t Size(const std::string &channel_name);

  size_t Capacity(const std::string &channel_name);

 private:
  inline bool isCreated(const std::string &channel_name) const;

  DataQueueMgr(const DataQueueMgr &) = delete;
  DataQueueMgr &operator=(const DataQueueMgr &) = delete;

  bool init_;
  bool closed_;
  std::mutex mutex_;
  std::mutex close_mutex_;
  std::condition_variable cv_;
  // how many queues opened by dataset
  int open_by_dataset_;
  Semaphore sema;
  bool dynamic_shape_{false};
  size_t default_capacity_{2};

  std::map<std::string, std::shared_ptr<BlockingQueue>> name_queue_map_;

  std::map<std::string, DataQueueCreator> data_queue_creator_map_ = {};  // key: device name, value: DataQueueCreator
};
#ifndef BUILD_LITE
bool PopDataFromDataQueue(const AnfNodePtr &data_kernel);
#endif
#define REGISTER_DATA_QUEUE_CREATOR(device_name, creator)                         \
  struct device_name##DataQueueCreatorClass {                                     \
    device_name##DataQueueCreatorClass() {                                        \
      DataQueueMgr::GetInstance().RegisterDataQueueCreator(device_name, creator); \
    }                                                                             \
  } g_##device_name##_data_queue_creator;
}  // namespace device
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_INCLUDE_BACKEND_DATA_QUEUE_DATA_QUEUE_MGR_H
