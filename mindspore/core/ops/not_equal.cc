/**
 * Copyright 2020-2021 Huawei Technologies Co., Ltd
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

#include "ops/not_equal.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "ops/op_utils.h"
#include "utils/check_convert_utils.h"
#include "abstract/primitive_infer_map.h"

namespace mindspore {
namespace ops {
namespace {
abstract::ShapePtr InferShape(const PrimitivePtr &primitive, const std::vector<AbstractBasePtr> &input_args) {
  MS_EXCEPTION_IF_NULL(primitive);
  auto op_name = primitive->name();
  CheckAndConvertUtils::CheckInteger("input number", input_args.size(), kGreaterEqual, 2, op_name);
  for (const auto &item : input_args) {
    MS_EXCEPTION_IF_NULL(item);
  }
  return BroadCastInferShape(op_name, input_args);
}

TypePtr InferType(const PrimitivePtr &prim, const std::vector<AbstractBasePtr> &input_args) {
  MS_EXCEPTION_IF_NULL(prim);
  auto op_name = prim->name();
  CheckAndConvertUtils::CheckInteger("input number", input_args.size(), kGreaterEqual, 2, op_name);
  for (const auto &item : input_args) {
    MS_EXCEPTION_IF_NULL(item);
  }
  std::map<std::string, TypePtr> types;
  types.emplace("x", input_args[0]->BuildType());
  types.emplace("y", input_args[1]->BuildType());
  return CheckAndConvertUtils::CheckTensorTypeSame(types, common_valid_types, op_name);
}
}  // namespace

AbstractBasePtr NotEqualInfer(const abstract::AnalysisEnginePtr &, const PrimitivePtr &primitive,
                              const std::vector<AbstractBasePtr> &input_args) {
  (void)InferType(primitive, input_args);
  return abstract::MakeAbstract(InferShape(primitive, input_args), kBool);
}
REGISTER_PRIMITIVE_C(kNameNotEqual, NotEqual);
}  // namespace ops
}  // namespace mindspore
