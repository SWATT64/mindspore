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
#include "src/runtime/kernel/arm/fp32_grad/lstm_grad_fp32.h"
#include <string>
#include <memory>
#include <algorithm>
#include "utils/ms_utils.h"
#include "schema/model_generated.h"
#include "src/kernel_registry.h"
#include "nnacl/fp32/lstm_fp32.h"

namespace mindspore {
namespace kernel {
using mindspore::lite::KernelRegistrar;
using mindspore::lite::RET_ERROR;
using mindspore::lite::RET_OK;
using mindspore::schema::PrimitiveType_LSTMGrad;

int LSTMGradCPUKernel::Prepare() {
  CHECK_LESS_RETURN(in_tensors_.size(), DIMENSION_11D);
  CHECK_LESS_RETURN(out_tensors_.size(), 1);
  if (!InferShapeDone()) {
    return RET_OK;
  }
  return ReSize();
}

int LSTMGradCPUKernel::ReSize() { return InitParam(); }

int LSTMGradCPUKernel::Run() {
  auto ret = MallocRunBuffer();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "LstmGradCPUKernel MallocRunBuffer error.";
    FreeRunBuffer();
    return RET_ERROR;
  }
  LstmBackpropUnidirectional(false);
  FreeRunBuffer();
  return RET_OK;
}

int LSTMGradCPUKernel::LstmBackpropUnidirectional(bool is_backward) {
  // get input tensors
  auto dC_tensor = in_tensors_.at(dC_index);
  MS_ASSERT(dC_tensor != nullptr);
  auto dH_tensor = in_tensors_.at(dH_index);
  MS_ASSERT(dH_tensor != nullptr);
  auto dy_tensor = in_tensors_.at(dy_index);
  MS_ASSERT(dy_tensor != nullptr);
  auto input_tensor = in_tensors_.at(input_index);
  MS_ASSERT(input_tensor != nullptr);
  auto weights_tensor = in_tensors_.at(weights_index);
  MS_ASSERT(weights_tensor != nullptr);
  auto intermediate_tensor = in_tensors_.at(intermediate_data_index);
  MS_ASSERT(intermediate_tensor != nullptr);
  auto cell_input_tensor = in_tensors_.at(cell_input_index);
  MS_ASSERT(cell_input_tensor != nullptr);
  auto hidden_input_tensor = in_tensors_.at(hidden_input_index);
  MS_ASSERT(hidden_input_tensor != nullptr);

  // Get output tensors
  auto dW_tensor = out_tensors_.at(dW_out_index);
  MS_ASSERT(dW_tensor != nullptr);
  auto dX_tensor = out_tensors_.at(dX_out_index);
  MS_ASSERT(dX_tensor != nullptr);
  auto dH_out_tensor = out_tensors_.at(dH_out_index);
  MS_ASSERT(dH_out_tensor != nullptr);
  auto dC_out_tensor = out_tensors_.at(dC_out_index);
  MS_ASSERT(dC_out_tensor != nullptr);

  auto cell_input_data = reinterpret_cast<float *>(cell_input_tensor->data());
  auto hidden_input_data = reinterpret_cast<float *>(hidden_input_tensor->data());
  auto dh_out = reinterpret_cast<float *>(dH_out_tensor->data());
  auto dc_out = reinterpret_cast<float *>(dC_out_tensor->data());
  auto intermediate_data = reinterpret_cast<float *>(intermediate_tensor->data());
  auto dC = reinterpret_cast<float *>(dC_tensor->data());
  auto dH = reinterpret_cast<float *>(dH_tensor->data());
  auto dY = reinterpret_cast<float *>(dy_tensor->data());
  auto dW = reinterpret_cast<float *>(dW_tensor->data());
  auto dX = reinterpret_cast<float *>(dX_tensor->data());
  auto weights = reinterpret_cast<float *>(weights_tensor->data());
  auto input = reinterpret_cast<float *>(input_tensor->data());

  auto state_size = lstm_param_->batch_ * lstm_param_->hidden_size_;
  auto seq_stride = lstm_param_->seq_len_ * state_size;
  float *hidden_state = intermediate_data;
  float *cell_state = intermediate_data + seq_stride * 1;
  float *input_gate = intermediate_data + seq_stride * 2;
  float *output_gate = intermediate_data + seq_stride * 3;
  float *forget_gate = intermediate_data + seq_stride * 4;
  float *cell_gate = intermediate_data + seq_stride * 5;
  ReorderLstmWeightGrad(weights_tmp_, weights, getLstmOrderIFGO(), false);

  memset(dH, 0, dH_tensor->Size());
  memset(dC, 0, dC_tensor->Size());

  memset(dW_tmp_, 0, dW_tensor->Size());  // dW_tmp is summed in the loop
  float *workspace_gemm = workspace_ + GetRunWorkspaceGemmOffset(lstm_param_);

  for (int t = lstm_param_->seq_len_ - 1; t >= 0; t--) {
    int real_t = is_backward ? lstm_param_->seq_len_ - t - 1 : t;
    auto stride = real_t * state_size;

    float *prev_hidden_state = (real_t > 0) ? hidden_state + (real_t - 1) * state_size : hidden_input_data;
    float *curr_cell_state = cell_state + stride;
    float *prev_cell_state = (real_t > 0) ? cell_state + (real_t - 1) * state_size : cell_input_data;
    float *curr_input_gate = input_gate + stride;
    float *curr_forget_gate = forget_gate + stride;
    float *curr_cell_gate = cell_gate + stride;
    float *curr_output_gate = output_gate + stride;
    float *curr_input = input + real_t * lstm_param_->batch_ * lstm_param_->input_size_;
    float *curr_dx = dX + real_t * lstm_param_->batch_ * lstm_param_->input_size_;
    float *curr_dy = dY + real_t * state_size;

    float *dA = nullptr;
    LstmGradDoInputStep(curr_output_gate, curr_cell_state, prev_cell_state, curr_cell_gate, curr_input_gate,
                        curr_forget_gate, curr_dy, dC, dH, &dA, curr_dx, weights_tmp_, workspace_, lstm_param_);
    LstmGradDoWeightStep(curr_input, prev_hidden_state, dA, dW_tmp_, workspace_gemm, lstm_param_);
  }
  std::copy(&(dH[0]), &(dH[state_size]), &(dh_out[0]));
  std::copy(&(dC[0]), &(dC[state_size]), &(dc_out[0]));
  ReorderLstmWeightGrad(dW, dW_tmp_, getLstmOrderIOFG(), lstm_param_->has_bias_);
  return RET_OK;
}

void LSTMGradCPUKernel::ReorderLstmWeightGrad(float *dst, float *src, const int *order, bool include_bias) {
  ReorderLstmWeights(dst, src, weight_batch_, lstm_param_->hidden_size_, lstm_param_->input_size_, order);
  src += weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->input_size_;
  dst += weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->input_size_;
  ReorderLstmWeights(dst, src, weight_batch_, lstm_param_->hidden_size_, lstm_param_->hidden_size_, order);
  src += weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->hidden_size_;
  dst += weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->hidden_size_;
  if (include_bias) {
    ReorderLstmWeights(dst, src, weight_batch_, 1, lstm_param_->hidden_size_, order);
  }
}

int LSTMGradCPUKernel::DoGrad(int thread_id) { return RET_OK; }

int LSTMGradCPUKernel::InitParam() {
  auto input = in_tensors_.front();
  MS_ASSERT(input != nullptr);
  std::vector<int> in_shape = input->shape();
  lstm_param_->seq_len_ = in_shape.at(FIRST_INPUT);
  lstm_param_->batch_ = in_shape.at(SECOND_INPUT);
  lstm_param_->input_size_ = in_shape.at(THIRD_INPUT);

  auto dy = in_tensors_.at(dy_index);
  MS_ASSERT(dy != nullptr);
  std::vector<int> dy_shape = dy->shape();
  lstm_param_->hidden_size_ = dy_shape.at(THIRD_INPUT);

  int dir_multiplier = lstm_param_->bidirectional_ ? 2 : 1;
  lstm_param_->output_step_ = dir_multiplier * lstm_param_->batch_ * lstm_param_->hidden_size_;
  weight_batch_ = dir_multiplier * num_of_gates;
  state_is_vec_ = lstm_param_->batch_ == 1;

#ifdef ENABLE_AVX
  row_tile_ = C6NUM;
  col_tile_ = C16NUM;
#elif defined(ENABLE_ARM32)
  row_tile_ = C12NUM;
  col_tile_ = C4NUM;
#elif defined(ENABLE_SSE)
  row_tile_ = C4NUM;
  col_tile_ = C8NUM;
#else
  row_tile_ = C12NUM;
  col_tile_ = C8NUM;
#endif
  lstm_param_->input_row_align_ = UP_ROUND(lstm_param_->seq_len_ * lstm_param_->batch_, row_tile_);
  lstm_param_->input_col_align_ = UP_ROUND(lstm_param_->hidden_size_, col_tile_);
  input_size_align_ = UP_ROUND(lstm_param_->input_size_, row_tile_);
  input_thread_count_ = MSMIN(op_parameter_->thread_num_, UP_DIV(lstm_param_->input_col_align_, col_tile_));
  input_thread_stride_ = UP_DIV(UP_DIV(lstm_param_->input_col_align_, col_tile_), input_thread_count_);

  state_row_tile_ = row_tile_;
  state_col_tile_ = col_tile_;

  lstm_param_->state_row_align_ = state_is_vec_ ? 1 : UP_ROUND(lstm_param_->batch_, state_row_tile_);
  lstm_param_->state_col_align_ =
    state_is_vec_ ? lstm_param_->hidden_size_ : UP_ROUND(lstm_param_->hidden_size_, state_col_tile_);

  return RET_OK;
}
int LSTMGradCPUKernel::MallocRunBuffer() {
  int workspace_size = GetRunWorkspaceSize(lstm_param_);
  if ((workspace_size == 0) || (workspace_size > LSTMGRAD_MAX_WORKSPACE_SIZE)) {
    MS_LOG(ERROR) << "LstmCPUKernel malloc run workspace 0 error.";
    return RET_ERROR;
  }
  workspace_ = reinterpret_cast<float *>(ms_context_->allocator->Malloc(workspace_size * sizeof(float)));
  if (workspace_ == nullptr) {
    MS_LOG(ERROR) << "LstmCPUKernel malloc run workspace error.";
    return RET_ERROR;
  }
  auto dW_tensor = out_tensors_.at(dW_out_index);
  MS_ASSERT(dW_tensor != nullptr);
  auto dW_size = dW_tensor->Size();
  if ((dW_size == 0) || (dW_size > LSTMGRAD_MAX_WEIGHTS_SIZE)) {
    MS_LOG(ERROR) << "LstmCPUKernel malloc run dW_tmp size error.";
    return RET_ERROR;
  }
  dW_tmp_ = reinterpret_cast<float *>(ms_context_->allocator->Malloc(dW_size));
  if (dW_tmp_ == nullptr) {
    MS_LOG(ERROR) << "LstmCPUKernel malloc run dW_tmp alloc error.";
    return RET_ERROR;
  }
  int weights_size = weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->input_size_ +  // IW matrics
                     weight_batch_ * lstm_param_->hidden_size_ * lstm_param_->hidden_size_;  // V matrics
  weights_tmp_ = reinterpret_cast<float *>(ms_context_->allocator->Malloc(weights_size * sizeof(float)));
  if (weights_tmp_ == nullptr) {
    MS_LOG(ERROR) << "LstmCPUKernel malloc run weights_tmp_ alloc error.";
    return RET_ERROR;
  }

  return RET_OK;
}

void LSTMGradCPUKernel::FreeRunBuffer() {
  if (workspace_ != nullptr) {
    ms_context_->allocator->Free(workspace_);
    workspace_ = nullptr;
  }
  if (dW_tmp_ != nullptr) {
    ms_context_->allocator->Free(dW_tmp_);
    dW_tmp_ = nullptr;
  }
  if (weights_tmp_ != nullptr) {
    ms_context_->allocator->Free(weights_tmp_);
    weights_tmp_ = nullptr;
  }
}

REG_KERNEL(kCPU, kNumberTypeFloat32, PrimitiveType_LSTMGrad, LiteKernelCreator<LSTMGradCPUKernel>)
}  // namespace kernel
}  // namespace mindspore
