# Copyright 2020 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

"""
Primitive operator classes.

A collection of operators to build nerual networks or computing functions.
"""

from .array_ops import (Argmax, Argmin, Cast, ConcatOffset, Concat, Pack, Unpack,
                        Diag, DiagPart, DType, ExpandDims, Eye,
                        Fill, GatherNd, GatherV2, InvertPermutation,
                        IsInstance, IsSubClass, ArgMaxWithValue, OnesLike, ZerosLike,
                        Rank, Reshape, ResizeNearestNeighbor, ArgMinWithValue,
                        SameTypeShape,
                        ScalarToArray, ScalarToTensor, ScatterNd, ScatterNdUpdate, Select,
                        Shape, Size, Slice, Split,
                        Squeeze, StridedSlice, Tile,
                        Transpose, TruncatedNormal, TupleToArray,
                        UnsortedSegmentSum, SpaceToDepth, DepthToSpace, SpaceToBatch, BatchToSpace)
from .comm_ops import (AllGather, AllReduce, _AlltoAll, ReduceScatter, Broadcast,
                       _MirrorOperator, ReduceOp, _VirtualDataset,
                       _VirtualDiv, _GetTensorSlice)
from .debug_ops import (ImageSummary, InsertGradientOf, ScalarSummary,
                        TensorSummary, Print)
from .control_ops import ControlDepend, GeSwitch, Merge
from .inner_ops import ScalarCast
from .math_ops import (Abs, ACos, AddN, AssignAdd, AssignSub, Atan2, BatchMatMul,
                       ReduceMax, ReduceMin, ReduceMean, ReduceSum, ReduceAll, ReduceProd, CumProd,
                       Cos, Div, Equal, EqualCount, Exp, Floor, FloorDiv, FloorMod, Acosh,
                       Greater, GreaterEqual, Less, LessEqual, Log, LogicalAnd,
                       LogicalNot, LogicalOr, MatMul, Maximum,
                       Minimum, Mul, Neg, NMSWithMask, NotEqual,
                       NPUAllocFloatStatus, NPUClearFloatStatus,
                       NPUGetFloatStatus, Pow, RealDiv, IsNan, IsInf, IsFinite, FloatStatus,
                       Reciprocal, CumSum,
                       Sin, Sqrt, Rsqrt,
                       Square, Sub, TensorAdd, Sign, Round)
from .random_ops import (RandomChoiceWithMask)
from .nn_ops import (LSTM, SGD, Adam, ApplyMomentum, BatchNorm,
                     BiasAdd, Conv2D,
                     DepthwiseConv2dNative,
                     DropoutDoMask,
                     DropoutGenMask, Flatten, FusedBatchNorm,
                     Gelu, Elu,
                     GetNext, L2Normalize, LayerNorm,
                     LogSoftmax,
                     MaxPool, ExtractImagePatches,
                     AvgPool, Conv2DBackpropInput,
                     MaxPoolWithArgmax, OneHot, Pad, MirrorPad, PReLU, ReLU, ReLU6, HSwish, HSigmoid,
                     ResizeBilinear, Sigmoid,
                     SigmoidCrossEntropyWithLogits,
                     SmoothL1Loss, Softmax,
                     SoftmaxCrossEntropyWithLogits, ROIAlign,
                     SparseSoftmaxCrossEntropyWithLogits, Tanh,
                     TopK, BinaryCrossEntropy, SparseApplyAdagrad, LARSUpdate, ApplyFtrl,
                     ApplyRMSProp, ApplyCenteredRMSProp)
from .other_ops import Assign, IOU, BoundingBoxDecode, BoundingBoxEncode, CheckValid, MakeRefKey
from . import _quant_ops
from ._quant_ops import *

__all__ = [
    'TensorAdd',
    'Argmax',
    'Argmin',
    'ArgMaxWithValue',
    'ArgMinWithValue',
    'AddN',
    'Sub',
    'CumSum',
    'MatMul',
    'BatchMatMul',
    'Mul',
    'Pow',
    'Exp',
    'Rsqrt',
    'Sqrt',
    'Square',
    'Conv2D',
    'ExtractImagePatches',
    'Flatten',
    'MaxPoolWithArgmax',
    'FusedBatchNorm',
    'BatchNorm',
    'MaxPool',
    'TopK',
    'Adam',
    'Softmax',
    'LogSoftmax',
    'SoftmaxCrossEntropyWithLogits',
    'ROIAlign',
    'SparseSoftmaxCrossEntropyWithLogits',
    'SGD',
    'ApplyMomentum',
    'ExpandDims',
    'Cast',
    'IsSubClass',
    'IsInstance',
    'Reshape',
    'Squeeze',
    'Transpose',
    'OneHot',
    'GatherV2',
    'Concat',
    'Pack',
    'Unpack',
    'Tile',
    'BiasAdd',
    'Gelu',
    'Minimum',
    'Maximum',
    'StridedSlice',
    'ReduceSum',
    'ReduceMean',
    'LayerNorm',
    'Rank',
    'Less',
    'LessEqual',
    'RealDiv',
    'Div',
    'TruncatedNormal',
    'Fill',
    'OnesLike',
    'ZerosLike',
    'Select',
    'Split',
    'ReLU',
    'ReLU6',
    'Elu',
    'Sigmoid',
    'HSwish',
    'HSigmoid',
    'Tanh',
    'RandomChoiceWithMask',
    'ResizeBilinear',
    'ScalarSummary',
    'ImageSummary',
    'TensorSummary',
    "Print",
    'InsertGradientOf',
    'InvertPermutation',
    'Shape',
    'DropoutDoMask',
    'DropoutGenMask',
    'Neg',
    'Slice',
    'DType',
    'NPUAllocFloatStatus',
    'NPUGetFloatStatus',
    'NPUClearFloatStatus',
    'IsNan',
    'IsFinite',
    'IsInf',
    'FloatStatus',
    'Reciprocal',
    'SmoothL1Loss',
    'ReduceAll',
    'ScalarToArray',
    'ScalarToTensor',
    'TupleToArray',
    'ControlDepend',
    'GeSwitch',
    'Merge',
    'SameTypeShape',
    'CheckValid',
    'BoundingBoxEncode',
    'BoundingBoxDecode',
    'L2Normalize',
    'ScatterNd',
    'ResizeNearestNeighbor',
    'Pad',
    'MirrorPad',
    'GatherNd',
    'ScatterNdUpdate',
    'Floor',
    'NMSWithMask',
    'IOU',
    'MakeRefKey',
    'AvgPool',
    # Back Primitive
    'Equal',
    'EqualCount',
    'NotEqual',
    'Greater',
    'GreaterEqual',
    'LogicalNot',
    'LogicalAnd',
    'LogicalOr',
    'Size',
    'DepthwiseConv2dNative',
    'ConcatOffset',
    'UnsortedSegmentSum',
    "AllGather",
    "AllReduce",
    "ReduceScatter",
    "Broadcast",
    "ReduceOp",
    'ScalarCast',
    'GetNext',
    'ReduceMax',
    'ReduceMin',
    'ReduceProd',
    'CumProd',
    'Log',
    'SigmoidCrossEntropyWithLogits',
    'FloorDiv',
    'FloorMod',
    'Acosh',
    "PReLU",
    "Cos",
    "ACos",
    "Diag",
    "DiagPart",
    'Eye',
    'Assign',
    'AssignAdd',
    'AssignSub',
    "Sin",
    "LSTM",
    "Abs",
    "BinaryCrossEntropy",
    "SparseApplyAdagrad",
    "SpaceToDepth",
    "DepthToSpace",
    "Conv2DBackpropInput",
    "Sign",
    "LARSUpdate",
    "Round",
    "ApplyFtrl",
    "SpaceToBatch",
    "BatchToSpace",
    "Atan2",
    "ApplyRMSProp",
    "ApplyCenteredRMSProp"
]

__all__.extend(_quant_ops.__all__)
__all__.sort()
