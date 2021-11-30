/**
 * Copyright 2021 Huawei Technologies Co., Ltd
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

#ifndef MINDSPORE_CCSRC_RUNTIME_HARDWARE_CPU_NVIDIA_COLLECTIVE_COMM_LIB_H_
#define MINDSPORE_CCSRC_RUNTIME_HARDWARE_CPU_NVIDIA_COLLECTIVE_COMM_LIB_H_

#include <nccl.h>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include "runtime/hardware/collective/collective_communication_lib.h"
#include "runtime/hardware/gpu/nvidia_communication_group.h"

#ifndef EXPORT_NCCL_WRAPPER
#define EXPORT_NCCL_WRAPPER __attribute__((visibility("default")))
#endif

namespace mindspore {
namespace device {
namespace gpu {
// Map of collective operation data type to NCCL data type.
const std::map<TypeId, ncclDataType_t> kNCCLDataTypeMap = {
  {TypeId::kNumberTypeInt8, ncclChar},       {TypeId::kNumberTypeUInt8, ncclUint8},
  {TypeId::kNumberTypeInt32, ncclInt32},     {TypeId::kNumberTypeInt, ncclInt},
  {TypeId::kNumberTypeUInt32, ncclUint32},   {TypeId::kNumberTypeInt64, ncclInt64},
  {TypeId::kNumberTypeUInt64, ncclUint64},   {TypeId::kNumberTypeFloat16, ncclHalf},
  {TypeId::kNumberTypeFloat32, ncclFloat32}, {TypeId::kNumberTypeFloat, ncclFloat32},
  {TypeId::kNumberTypeFloat64, ncclFloat64}};

// Map of reduce type to NCCL reduce type.
const std::map<ReduceMode, ncclRedOp_t> kNCCLReduceTypeMap = {
  {Reduce_Sum, ncclSum}, {Reduce_Prod, ncclProd}, {Reduce_Min, ncclMin}, {Reduce_Max, ncclMax}};

constexpr char kNCCLGlobalGroupName[] = "nccl_world_group";

class EXPORT_NCCL_WRAPPER NvidiaCollectiveCommLib : public CollectiveCommunicationLib {
 public:
  static NvidiaCollectiveCommLib &GetInstance() {
    static NvidiaCollectiveCommLib instance;
    return instance;
  }

  bool Initialize(uint32_t global_rank = UINT32_MAX, uint32_t global_rank_size = UINT32_MAX) override;

  bool CreateCommunicationGroup(const std::string &group_name, const std::vector<uint32_t> &group_ranks) override;

  bool AllGather(const void *send_buff, void *recv_buff, size_t send_count, TypeId data_type,
                 const std::string &group_name, void *stream = nullptr) override;

  bool AllReduce(const void *send_buff, void *recv_buff, size_t send_count, TypeId data_type, ReduceMode reduce_op,
                 const std::string &group_name, void *stream = nullptr) override;

  bool Broadcast(const void *send_buff, void *recv_buff, size_t send_count, TypeId data_type, uint32_t root_rank,
                 const std::string &group_name, void *stream = nullptr) override;

  bool ReduceScatter(const void *send_buff, void *recv_buff, size_t recv_count, TypeId data_type, ReduceMode reduce_op,
                     const std::string &group_name, void *stream = nullptr) override;

  bool Send(const void *send_buff, size_t count, TypeId data_type, uint32_t peer, const std::string &group_name,
            void *stream = nullptr) override;

  bool Recv(void *recv_buff, size_t count, TypeId data_type, uint32_t peer, const std::string &group_name,
            void *stream = nullptr) override;

 private:
  NvidiaCollectiveCommLib();
  ~NvidiaCollectiveCommLib() override = default;

  // Check data type of collective operation is valid for NCCL.
  bool CheckNCCLDataType(TypeId data_type);

  // Check reduce type of collective operation is valid for NCCL.
  bool CheckNCCLReduceType(ReduceMode reduce_op);
};
}  // namespace gpu

extern "C" EXPORT_NCCL_WRAPPER CollectiveCommunicationLib *communication_lib_instance();
}  // namespace device
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_RUNTIME_HARDWARE_CPU_NVIDIA_COLLECTIVE_COMM_LIB_H_
