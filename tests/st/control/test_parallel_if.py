# Copyright 2022 Huawei Technologies Co., Ltd
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
import mindspore.context as context
from mindspore import Tensor, ms_function
from mindspore.common import dtype as mstype


def setup_module():
    context.set_context(mode=context.GRAPH_MODE)


def test_while_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in while loop requires that the after-if func graph should not
                 be called.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                if y > 0:
                    return bias
                x = x - 1
                y = y + 1
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_break_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                if y > 0:
                    return bias
                break
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_return_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                if y > 0:
                    return bias
                return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_break_else_return_in_while_in_else_take_break():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: take the break branch, success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                if y > 0:
                    break
                return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_break_else_return_in_while_in_else_take_return():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: take the return branch, success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                if y < 0:
                    break
                return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([4], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_while_return_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through outer while to
                 the else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                while x > 0:
                    return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_in_while_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through outer while to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                while x > 0:
                    if y > 0:
                        return bias
                    x = x - 1
                    y = y + 1
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_return_in_while_in_while_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                while x > 0:
                    if y > 0:
                        return bias
                    return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_while_return_after_if_else_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part. The inner if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            if x > y:
                x = x + y
            else:
                x = x - y

            while x > 0:
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_while_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part. The inner if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                return bias
            if x > y:
                x = x + y
            else:
                x = x - y

        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_after_if_else_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner second if requires that the after-if func graph should not
                 be called, and this information should be propagated through if to
                 the outer else part. The inner first if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            if x > y:
                x = x + y
            else:
                x = x - y

            if x > 0:
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_if_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner second if requires that the after-if func graph should not
                 be called, and this information should be propagated through if to
                 the outer else part. The inner second if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            if x > 0:
                return bias
            if x > y:
                x = x + y
            else:
                x = x - y
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_while_return_in_else_after_if_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part. The first if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if x > y:
            x = x + y
        else:
            x = x - y

        if bias > y:
            y = x + y
        else:
            while x > 0:
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_by_while_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part. The second if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            while x > 0:
                return bias
        if x > y:
            x = x + y
        else:
            x = x - y
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_in_else_after_if_else():
    """
    Feature: Parallel if transformation.
    Description: return in else of the second if/else requires that the after-if func graph should not
                 be called, and this information should be propagated through if to
                 the outer else part. The first if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if x > y:
            x = x + y
        else:
            x = x - y
        if bias > y:
            y = x + y
        else:
            if x > 0:
                return bias

        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_by_if_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner second if requires that the after-if func graph should not
                 be called, and this information should be propagated through if to
                 the outer else part. The second first if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            if x > 0:
                return bias
        if x > y:
            x = x + y
        else:
            x = x - y

        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_in_if_while_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner while loop requires that the after-if func graph should not
                 be called, and this information should be propagated through while to
                 the outer else part. The inner if/else in the first if can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
            if x > y:
                x = x + y
            else:
                x = x - y
        else:
            while x > 0:
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_in_if_if_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in else of the first if/else requires that the after-if func graph should not
                 be called, and this information should be propagated through if to
                 the outer else part. The if/else inside the first if can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
            if x > y:
                x = x + y
            else:
                x = x - y
        else:
            if x > 0:
                return bias

        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_for_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in for loop requires that the after-if func graph should not
                 be called.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                if y > 0:
                    return bias
                x = x - 1
                y = y + 1
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_break_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                if y > 0:
                    return bias
                break
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_return_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                if y > 0:
                    return bias
                return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_for_return_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through outer for to
                 the else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                for _ in range(5):
                    return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_in_for_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through outer for to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                for _ in range(5):
                    if y > 0:
                        return bias
                    x = x - 1
                    y = y + 1
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_return_else_return_in_for_in_for_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                for _ in range(5):
                    if y > 0:
                        return bias
                    return x
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_for_return_after_if_else_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part. The inner if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            if x > y:
                x = x + y
            else:
                x = x - y

            for _ in range(5):
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_for_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part. The inner if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                return bias
            if x > y:
                x = x + y
            else:
                x = x - y

        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_for_return_in_else_after_if_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part. The first if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if x > y:
            x = x + y
        else:
            x = x - y

        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_after_by_for_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part. The second if/else can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
        else:
            for _ in range(5):
                return bias
        if x > y:
            x = x + y
        else:
            x = x - y
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect


def test_if_else_in_if_for_return_in_else():
    """
    Feature: Parallel if transformation.
    Description: return in inner for loop requires that the after-if func graph should not
                 be called, and this information should be propagated through for to
                 the outer else part. The inner if/else in the first if can be transformed.
    Expectation: success
    """

    @ms_function
    def foo(x, y, bias):
        if bias > y:
            y = x + y
            if x > y:
                x = x + y
            else:
                x = x - y
        else:
            for _ in range(5):
                return bias
        return x + y

    x = Tensor([4], mstype.int32)
    y = Tensor([1], mstype.int32)
    bias = Tensor([-5], mstype.int32)
    expect = Tensor([-5], mstype.int32)
    ret = foo(x, y, bias)
    assert ret == expect
