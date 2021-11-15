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

#ifndef MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_CONTROLFLOW_CONTROL_ACTOR_H_
#define MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_CONTROLFLOW_CONTROL_ACTOR_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <stack>
#include <queue>
#include <utility>
#include "runtime/framework/actor/actor_common.h"
#include "runtime/framework/actor/abstract_actor.h"
#include "runtime/framework/actor/memory_aware_actor.h"

namespace mindspore {
namespace runtime {
// Op data with branch ID represents the data sent by gather actor to entrance actor, including all real
// parameters and the id of the caller.
using OpDataWithBranchID = std::pair<std::vector<DeviceTensor *>, int>;
// Op partial represents the partial structure, including a funcgraph and its real parameters.
using OpPartial = std::pair<FuncGraph *, std::vector<DeviceTensor *>>;
// The control actor is the base class of control flow actor.
class ControlActor : public AbstractActor {
 public:
  ControlActor(const std::string &name, KernelTransformType type, const std::vector<KernelWithIndex> &parameters,
               const AnfNodePtr &node);
  ~ControlActor() override = default;

  void Init() override;

  // Receive partial.
  virtual void RunOpPartial(FuncGraph *func_graph, std::vector<DeviceTensor *> input_data, size_t position,
                            OpContext<DeviceTensor> *const context);

  // Receive branch id.
  virtual void RunBranchID(int branch_id, OpContext<DeviceTensor> *const context);

  const std::vector<DataArrowPtr> &output_partial_arrows() const { return output_partial_arrows_; }
  const std::vector<AID> &output_branch_id_arrows() const { return output_branch_id_arrows_; }
  const std::unordered_map<size_t, OpPartial> &local_partials() const { return local_partials_; }
  const std::vector<AID> &input_partial_arrow_aids() const { return input_partial_arrow_aids_; }
  const std::vector<AID> &input_branch_id_arrow_aids() const { return input_branch_id_arrow_aids_; }

 protected:
  friend class ControlNodeScheduler;
  // Get the position of node in the input.
  size_t FetchNodePosition(const KernelWithIndex &node) const;

  // Get all input, including data, partial, branchid.
  virtual void FetchInput(OpContext<DeviceTensor> *const context);
  void Run(OpContext<DeviceTensor> *const context) override;
  bool CheckRunningCondition(const OpContext<DeviceTensor> *context) const override;
  void SendOutput(OpContext<DeviceTensor> *const context) override;
  void EraseInput(const OpContext<DeviceTensor> *context) override;

  // Input data.
  // 1.Input partial.
  // Record the partial received by each step, the key of the pair indicates the location of the partial.
  std::unordered_map<int, std::vector<std::pair<size_t, OpPartial>>> input_op_partials_;
  // 2. Branch ids is used to record the id corresponding to the output branch.
  // In control flow, sub funcgraph may be called in multiple places, and the output must be return to different
  // places. Therefore, the output of each subgraph will be connected to a exit actor, and the caller will send
  // its branch id to the entrance actor of the subgraph. Then branch id will be sent by the entrance actor to
  // the exit actor connected to the output.
  // In a recursive scenario, the exit will sequentially receive the branch ids sent by the caller, and the exit
  // actor needs to store the branch ids in the stack, and pop up in turn when returning.
  std::unordered_map<int, std::stack<int>> input_branch_ids_;

  // Fetch data. After fetch input, all the input collected is saved here.
  std::vector<OpPartial> input_partials_;
  std::vector<DeviceTensor *> input_device_tensors_;

  // Input num.
  size_t input_partials_num_{0};

  // The dependent input actors.
  std::vector<AID> input_partial_arrow_aids_;
  std::vector<AID> input_branch_id_arrow_aids_;

  // Output Arrows.
  std::vector<DataArrowPtr> output_partial_arrows_;
  OpPartial output_partial_;

  std::vector<AID> output_branch_id_arrows_;
  // The branch id is the unique identifier of the control actor. In the control flow, there are multiple control
  // actors calling the same subgraph at the same time. At this time, the output of the subgraph needs to be returned
  // to the calling place according to the branch id.
  int output_branch_id_;

  // Partial data in local. When partial is only funcgraph without real parameter, it is stored inside the actor.
  std::unordered_map<size_t, OpPartial> local_partials_;
  // Device tensor in control node, but not in kernel graph.
  std::unordered_map<size_t, DeviceTensor *> local_device_tensors_;

  // Cache output data by output index to modify the output data effectively.
  std::vector<std::vector<OpData<DeviceTensor> *>> output_data_by_output_index_;

  // Formal parameters for control actor.
  std::vector<KernelWithIndex> formal_parameters_;
  // local node for control actor, such as return node for exit actor, switch node for switch actor.
  AnfNodePtr node_;
};

using ControlActorPtr = std::shared_ptr<ControlActor>;
}  // namespace runtime
}  // namespace mindspore

#endif  // MINDSPORE_CCSRC_RUNTIME_FRAMEWORK_ACTOR_CONTROLFLOW_CONTROL_ACTOR_H_
