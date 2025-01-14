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

#define USE_DEPRECATED_API
#include "tools/optimizer/fusion/multi_head_attention_fusion.h"
#include <functional>
#include <utility>
#include <vector>
#include "tools/optimizer/common/gllo_utils.h"
#include "nnacl/op_base.h"
#include "tools/common/tensor_util.h"

namespace mindspore::opt {
namespace {
const auto &p1 = std::placeholders::_1;
const size_t kWeightShapeSize = 2;
}  // namespace

namespace {
VectorRef DefineEmbedding(const BaseRef &input, const BaseRef &weight, const BaseRef &bias, bool test_div = false) {
  auto is_matmul = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul != nullptr, {});
  auto dense = VectorRef({is_matmul, input, weight, bias});
  auto is_reshape = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimReshape));
  MS_CHECK_TRUE_RET(is_reshape != nullptr, {});
  auto var1 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var1 != nullptr, {});
  auto reshape = VectorRef({is_reshape, dense, var1});
  auto is_transpose = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimTranspose));
  MS_CHECK_TRUE_RET(is_transpose != nullptr, {});
  auto var2 = std::make_shared<Var>();
  auto transpose = VectorRef({is_transpose, reshape, var2});
  if (test_div) {
    auto is_div = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimRealDiv));
    MS_CHECK_TRUE_RET(is_div != nullptr, {});
    auto var3 = std::make_shared<Var>();
    MS_CHECK_TRUE_RET(var3 != nullptr, {});
    auto div = VectorRef({is_div, transpose, var3});
    return div;
  }
  return transpose;
}

VectorRef DefineMask(const BaseRef &mask_input) {
  auto is_expand_dims = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimExpandDims));
  MS_CHECK_TRUE_RET(is_expand_dims != nullptr, {});
  auto var1 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var1 != nullptr, {});
  auto expand_dims = VectorRef({is_expand_dims, mask_input, var1});
  auto is_sub = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimSubFusion));
  MS_CHECK_TRUE_RET(is_sub != nullptr, {});
  auto var2 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var2 != nullptr, {});
  auto sub = VectorRef({is_sub, var2, expand_dims});
  auto is_mul = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMulFusion));
  MS_CHECK_TRUE_RET(is_mul != nullptr, {});
  auto var3 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var3 != nullptr, {});
  return VectorRef({is_mul, sub, var3});
}
}  // namespace

VectorRef MultiHeadAttentionFusion::DefineMPWithMaskPattern(bool cross, bool mask) const {
  VectorRef k_embedding, v_embedding;
  auto q_embedding = DefineEmbedding(input_q_, weight_q_, bias_q_, true);
  MS_CHECK_TRUE_RET(!q_embedding.empty(), {});
  if (!cross) {
    k_embedding = DefineEmbedding(input_q_, weight_k_, bias_k_, true);
    MS_CHECK_TRUE_RET(!k_embedding.empty(), {});
    v_embedding = DefineEmbedding(input_q_, weight_v_, bias_v_);
    MS_CHECK_TRUE_RET(!v_embedding.empty(), {});
  } else {
    k_embedding = DefineEmbedding(input_k_, weight_k_, bias_k_, true);
    MS_CHECK_TRUE_RET(!k_embedding.empty(), {});
    v_embedding = DefineEmbedding(input_k_, weight_v_, bias_v_);
    MS_CHECK_TRUE_RET(!v_embedding.empty(), {});
  }
  auto is_matmul1 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul1 != nullptr, {});
  auto is_reshape1 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimReshape));
  MS_CHECK_TRUE_RET(is_reshape1 != nullptr, {});
  auto matmul1 = VectorRef({is_matmul1, q_embedding, k_embedding});
  auto var1 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var1 != nullptr, {});
  VectorRef reshape1;
  if (mask) {
    auto is_add = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimAddFusion));
    MS_CHECK_TRUE_RET(is_add != nullptr, {});
    auto mask = DefineMask(mask_);
    MS_CHECK_TRUE_RET(!mask.empty(), {});
    auto add = VectorRef({is_add, mask, matmul1});
    reshape1 = VectorRef({is_reshape1, add, var1});
  } else {
    reshape1 = VectorRef({is_reshape1, matmul1, var1});
  }
  auto is_softmax = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimSoftmax));
  MS_CHECK_TRUE_RET(is_softmax != nullptr, {});
  auto softmax = VectorRef({is_softmax, reshape1});
  auto is_reshape2 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimReshape));
  MS_CHECK_TRUE_RET(is_reshape2 != nullptr, {});
  auto var2 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var2 != nullptr, {});
  auto reshape2 = VectorRef({is_reshape2, softmax, var2});
  auto is_matmul2 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul2 != nullptr, {});
  auto matmul2 = VectorRef({is_matmul2, reshape2, v_embedding});
  auto is_transpose = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimTranspose));
  MS_CHECK_TRUE_RET(is_transpose != nullptr, {});
  auto var3 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var3 != nullptr, {});
  auto transpose = VectorRef({is_transpose, matmul2, var3});
  auto is_reshape3 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimReshape));
  MS_CHECK_TRUE_RET(is_reshape3 != nullptr, {});
  auto var4 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var4 != nullptr, {});
  auto reshape3 = VectorRef({is_reshape3, transpose, var4});
  auto is_matmul3 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul3 != nullptr, {});
  auto matmul3 = VectorRef({is_matmul3, reshape3, weight_o_, bias_o_});
  return matmul3;
}

VectorRef MultiHeadAttentionFusion::DefineMPWithMaskPatternPA(bool cross) const {
  VectorRef k_embedding, v_embedding;
  auto q_embedding = DefineEmbedding(input_q_, weight_q_, bias_q_, true);
  MS_CHECK_TRUE_RET(!q_embedding.empty(), {});
  if (!cross) {
    k_embedding = DefineEmbedding(input_q_, weight_k_, bias_k_, true);
    MS_CHECK_TRUE_RET(!k_embedding.empty(), {});
    v_embedding = DefineEmbedding(input_q_, weight_v_, bias_v_);
    MS_CHECK_TRUE_RET(!v_embedding.empty(), {});
  } else {
    k_embedding = DefineEmbedding(input_k_, weight_k_, bias_k_, true);
    MS_CHECK_TRUE_RET(!k_embedding.empty(), {});
    v_embedding = DefineEmbedding(input_k_, weight_v_, bias_v_);
    MS_CHECK_TRUE_RET(!v_embedding.empty(), {});
  }
  auto is_matmul1 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul1 != nullptr, {});
  auto matmul1 = VectorRef({is_matmul1, q_embedding, k_embedding});
  auto var1 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var1 != nullptr, {});
  auto is_add = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimAddFusion));
  MS_CHECK_TRUE_RET(is_add != nullptr, {});
  auto mask = DefineMask(mask_);
  MS_CHECK_TRUE_RET(!mask.empty(), {});
  auto add = VectorRef({is_add, mask, matmul1});
  auto is_softmax = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimSoftmax));
  MS_CHECK_TRUE_RET(is_softmax != nullptr, {});
  auto softmax = VectorRef({is_softmax, add});
  auto is_matmul2 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul2 != nullptr, {});
  auto matmul2 = VectorRef({is_matmul2, softmax, v_embedding});
  auto is_transpose = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimTranspose));
  MS_CHECK_TRUE_RET(is_transpose != nullptr, {});
  auto var3 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var3 != nullptr, {});
  auto transpose = VectorRef({is_transpose, matmul2, var3});
  auto is_reshape3 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimReshape));
  MS_CHECK_TRUE_RET(is_reshape3 != nullptr, {});
  auto var4 = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(var4 != nullptr, {});
  auto reshape3 = VectorRef({is_reshape3, transpose, var4});
  auto is_matmul3 = std::make_shared<CondVar>(std::bind(IsOpType, p1, prim::kPrimMatMulFusion));
  MS_CHECK_TRUE_RET(is_matmul3 != nullptr, {});
  auto matmul3 = VectorRef({is_matmul3, reshape3, weight_o_, bias_o_});
  return matmul3;
}
namespace {
std::shared_ptr<tensor::Tensor> ConcatTensors(const std::vector<std::shared_ptr<tensor::Tensor>> &tensors) {
  const std::vector<int64_t> &base_shape = tensors.at(0)->shape();
  auto base_shape_size = base_shape.size();
  auto base_data_type = tensors.at(0)->data_type();
  auto res =
    std::all_of(tensors.begin() + 1, tensors.end(),
                [&base_shape_size, &base_shape, &base_data_type](const std::shared_ptr<tensor::Tensor> &tensor) {
                  if (tensor->shape().size() != base_shape_size) {
                    return false;
                  }
                  auto &shape = tensor->shape();
                  for (std::size_t i = 1; i < shape.size(); ++i) {
                    if (shape.at(i) != base_shape.at(i)) return false;
                  }
                  if (tensor->data_type() != base_data_type) {
                    return false;
                  }
                  return true;
                });
  MS_CHECK_TRUE_RET(res, nullptr);
  // calculate shape
  std::vector<int64_t> new_shape;
  auto sum = std::accumulate(tensors.begin(), tensors.end(), 0,
                             [](int sum, const tensor::TensorPtr &tensor) { return sum + tensor->shape().at(0); });
  new_shape.push_back(sum);
  for (std::size_t i = 1; i < base_shape_size; i++) {
    new_shape.push_back(base_shape.at(i));
  }
  // calculate data
  auto concat_tensor = std::make_shared<tensor::Tensor>(base_data_type, new_shape);
  MS_CHECK_TRUE_RET(concat_tensor != nullptr, nullptr);
  std::size_t offset = 0;
  for (const auto &tensor : tensors) {
    void *ptr = reinterpret_cast<uint8_t *>(concat_tensor->data_c()) + offset;
    memcpy_s(ptr, concat_tensor->Size() - offset, tensor->data_c(), tensor->Size());
    offset += tensor->Size();
  }
  return concat_tensor;
}
}  // namespace

bool MultiHeadAttentionFusion::Init() const {
  input_q_ = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(input_q_ != nullptr, false);
  input_k_ = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(input_k_ != nullptr, false);
  // input_v_ = std::make_shared<Var>();
  // MS_CHECK_TRUE_RET(input_v_ != nullptr, false);

  weight_q_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(weight_q_ != nullptr, false);
  weight_k_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(weight_k_ != nullptr, false);
  weight_v_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(weight_v_ != nullptr, false);
  weight_o_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(weight_o_ != nullptr, false);

  bias_q_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(bias_q_ != nullptr, false);
  bias_k_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(bias_k_ != nullptr, false);
  bias_v_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(bias_v_ != nullptr, false);
  bias_o_ = std::make_shared<CondVar>(IsParamNode);
  MS_CHECK_TRUE_RET(bias_o_ != nullptr, false);

  mask_ = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(mask_ != nullptr, false);

  reshape_k_ = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(reshape_k_ != nullptr, false);
  reshape_v_ = std::make_shared<Var>();
  MS_CHECK_TRUE_RET(reshape_v_ != nullptr, false);
  return true;
}

std::unordered_map<std::string, VectorRef> MultiHeadAttentionFusion::DefinePatterns() const {
  std::unordered_map<std::string, VectorRef> patterns;
  if (!Init()) {
    MS_LOG(ERROR) << "initial member failed.";
    return patterns;
  }
  patterns[kMPAWithMaskPatternName] = DefineMPWithMaskPattern();
  patterns[kMPAXWithMaskPatternName] = DefineMPWithMaskPattern(true);
  patterns[kMPAPatternName] = DefineMPWithMaskPattern(false, false);
  patterns[kMPAXPatternName] = DefineMPWithMaskPattern(true, false);
  patterns[kMPAWithMaskPatternNamePA] = DefineMPWithMaskPatternPA();
  patterns[kMPAXWithMaskPatternNamePA] = DefineMPWithMaskPatternPA(true);
  return patterns;
}

AnfNodePtr MultiHeadAttentionFusion::Process(const std::string &pattern_name, const mindspore::FuncGraphPtr &func_graph,
                                             const mindspore::AnfNodePtr &node,
                                             const mindspore::EquivPtr &equiv) const {
  if (func_graph == nullptr || node == nullptr || equiv == nullptr) {
    return nullptr;
  }
  if ((pattern_name == kMPAWithMaskPatternName) || (pattern_name == kMPAWithMaskPatternNamePA)) {
    return CreateMaskedMultiHeadAttentionNode(func_graph, equiv, node->fullname_with_scope());
  } else if ((pattern_name == kMPAXWithMaskPatternName) || (pattern_name == kMPAXWithMaskPatternNamePA)) {
    return CreateMaskedMultiHeadAttentionNode(func_graph, equiv, node->fullname_with_scope(), true);
  } else if (pattern_name == kMPAPatternName) {
    return CreateMaskedMultiHeadAttentionNode(func_graph, equiv, node->fullname_with_scope(), false, false);
  } else if (pattern_name == kMPAXPatternName) {
    return CreateMaskedMultiHeadAttentionNode(func_graph, equiv, node->fullname_with_scope(), true, false);
  }

  { return nullptr; }
}

STATUS GetIntParameterData(const ParameterPtr &param_ptr, std::vector<int> *result) {
  if (param_ptr == nullptr || !param_ptr->has_default()) {
    MS_LOG(DEBUG) << "param not have default";
    return RET_ERROR;
  }
  auto default_param = param_ptr->default_param();
  if (default_param == nullptr || !utils::isa<tensor::TensorPtr>(default_param)) {
    MS_LOG(DEBUG) << "tensor_info is not tensor::TensorPtr";
    return RET_ERROR;
  }
  auto default_param_ptr = utils::cast<tensor::TensorPtr>(default_param);
  if (default_param_ptr->data_type() != kNumberTypeInt32 && default_param_ptr->data_type() != kNumberTypeInt) {
    MS_LOG(DEBUG) << "default param is not int";
    return RET_ERROR;
  }
  auto ptr = reinterpret_cast<int *>(default_param_ptr->data_c());
  int64_t shape_size =
    std::accumulate(default_param_ptr->shape().begin(), default_param_ptr->shape().end(), 1, std::multiplies<>());
  for (int64_t i = 0; i < shape_size; i++) {
    result->emplace_back(ptr[i]);
  }
  return RET_OK;
}

std::shared_ptr<ops::Attention> MultiHeadAttentionFusion::BuildAttentionPrim(const EquivPtr &equiv) const {
  MS_ASSERT(equiv != nullptr);
  auto attention_prim = std::make_shared<ops::Attention>();
  if (attention_prim == nullptr) {
    MS_LOG(ERROR) << "Build attention primitive failed.";
    return attention_prim;
  }
  if (!utils::isa<ParameterPtr>((*equiv)[reshape_k_])) {
    MS_LOG(ERROR) << "Reshape k is not a parameter";
    return nullptr;
  }

  if (!utils::isa<ParameterPtr>((*equiv)[reshape_v_])) {
    MS_LOG(ERROR) << "Reshape k is not a parameter";
    return nullptr;
  }

  auto reshape_k = utils::cast<ParameterPtr>((*equiv)[reshape_k_]);
  std::vector<int> shape_k;
  if (RET_OK != GetIntParameterData(reshape_k, &shape_k)) {
    MS_LOG(ERROR) << "Get reshape k data failed";
    return nullptr;
  }

  auto reshape_v = utils::cast<ParameterPtr>((*equiv)[reshape_v_]);
  std::vector<int> shape_v;
  if (RET_OK != GetIntParameterData(reshape_v, &shape_v)) {
    MS_LOG(ERROR) << "Get reshape k data failed";
    return nullptr;
  }
  if (shape_k.size() < kWeightShapeSize || shape_v.size() < kWeightShapeSize ||
      shape_k.at(shape_k.size() - kWeightShapeSize) != shape_v.at(shape_v.size() - kWeightShapeSize)) {
    MS_LOG(ERROR) << "Shape k or shape v is invalid.";
    return nullptr;
  }
  return attention_prim;
}

CNodePtr MultiHeadAttentionFusion::CreateMaskedMultiHeadAttentionNode(const FuncGraphPtr &func_graph,
                                                                      const EquivPtr &equiv, const string &base_name,
                                                                      bool cross, bool mask) const {
  MS_ASSERT(func_graph != nullptr);
  MS_ASSERT(equiv != nullptr);
  auto attention_prim = std::make_shared<ops::Attention>();
  if (attention_prim == nullptr) {
    MS_LOG(ERROR) << "Build attention primitive failed.";
    return nullptr;
  }
  auto attention_prim_c = attention_prim->GetPrim();
  MS_CHECK_TRUE_RET(attention_prim_c != nullptr, nullptr);
  auto value_node = NewValueNode(attention_prim_c);
  MS_CHECK_TRUE_RET(value_node != nullptr, nullptr);
  auto input_q = utils::cast<AnfNodePtr>((*equiv)[input_q_]);
  AnfNodePtr input_k, input_mask;

  if (cross) {
    input_k = utils::cast<AnfNodePtr>((*equiv)[input_k_]);
  }
  // auto input_v = utils::cast<AnfNodePtr>((*equiv)[input_v_]);

  auto weight_q = utils::cast<AnfNodePtr>((*equiv)[weight_q_]);
  auto weight_k = utils::cast<AnfNodePtr>((*equiv)[weight_k_]);
  auto weight_v = utils::cast<AnfNodePtr>((*equiv)[weight_v_]);
  auto weight_o = utils::cast<AnfNodePtr>((*equiv)[weight_o_]);

  auto bias_q = utils::cast<AnfNodePtr>((*equiv)[bias_q_]);
  auto bias_k = utils::cast<AnfNodePtr>((*equiv)[bias_k_]);
  auto bias_v = utils::cast<AnfNodePtr>((*equiv)[bias_v_]);
  auto bias_o = utils::cast<AnfNodePtr>((*equiv)[bias_o_]);
  if (mask) {
    input_mask = utils::cast<AnfNodePtr>((*equiv)[mask_]);
  }
  std::shared_ptr<tensor::Tensor> weight_q_tensor = GetTensorInfo(weight_q);
  std::shared_ptr<tensor::Tensor> weight_k_tensor = GetTensorInfo(weight_k);
  std::shared_ptr<tensor::Tensor> weight_v_tensor = GetTensorInfo(weight_v);
  std::shared_ptr<tensor::Tensor> bias_q_tensor = GetTensorInfo(bias_q);
  std::shared_ptr<tensor::Tensor> bias_k_tensor = GetTensorInfo(bias_k);
  std::shared_ptr<tensor::Tensor> bias_v_tensor = GetTensorInfo(bias_v);
  tensor::TensorPtr c_weights;
  if (cross) {
    c_weights = ConcatTensors({weight_k_tensor, weight_v_tensor});
  } else {
    c_weights = ConcatTensors({weight_q_tensor, weight_k_tensor, weight_v_tensor});
  }
  auto c_bias = ConcatTensors({bias_q_tensor, bias_k_tensor, bias_v_tensor});
  // convert tensors to params
  auto c_weight_param = func_graph->add_parameter();
  MS_CHECK_TRUE_RET(c_weight_param != nullptr, nullptr);
  if (lite::InitParameterFromTensorInfo(c_weight_param, c_weights) != lite::RET_OK) {
    MS_LOG(ERROR) << "Init parameter from tensor info failed.";
    return nullptr;
  }
  c_weight_param->set_name(base_name + "/weight_qkv");
  auto c_bias_param = func_graph->add_parameter();
  MS_CHECK_TRUE_RET(c_bias_param != nullptr, nullptr);
  if (lite::InitParameterFromTensorInfo(c_bias_param, c_bias) != lite::RET_OK) {
    MS_LOG(ERROR) << "Init parameter from tensor info failed.";
    return nullptr;
  }
  c_bias_param->set_name(base_name + "/bias_qkv");
  std::vector<AnfNodePtr> new_node_inputs;
  if (cross) {
    new_node_inputs = {value_node, input_q, input_k, input_k, weight_q, c_weight_param, weight_o, c_bias_param, bias_o};
  } else {
    new_node_inputs = {value_node, input_q, input_q, input_q, c_weight_param, weight_o, c_bias_param, bias_o};
  }
  if (mask) {
    new_node_inputs.push_back(input_mask);
  }
  auto new_node = func_graph->NewCNode(new_node_inputs);
  MS_CHECK_TRUE_RET(new_node != nullptr, nullptr);
  new_node->set_fullname_with_scope(base_name);
  return new_node;
}
}  // namespace mindspore::opt
