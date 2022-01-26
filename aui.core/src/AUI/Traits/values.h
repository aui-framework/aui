/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <type_traits>
#include <cassert>
#include <utility>

namespace aui {
    /**
     * Null-checking wrapper when usage of null is fatal.
     * @tparam T any pointer or pointer-like type
     */
    template<typename T>
    class assert_not_used_when_null {
    private:
        T mValue;

    public:
        assert_not_used_when_null(T value) noexcept: mValue(std::move(value)) {}

        template<typename AnyType>
        operator AnyType() noexcept {
            assert(("value is used when null" && mValue != nullptr));
            return reinterpret_cast<AnyType>(mValue);
        }

        template<typename AnyType>
        bool operator==(const AnyType& v) const noexcept {
            return mValue == v;
        }

        template<typename AnyType>
        bool operator!=(const AnyType& v) const noexcept {
            return mValue != v;
        }
    };


    template<typename T>
    struct non_null_lateinit {
    private:
        void checkForNull() const { assert(("this value couldn't be null" && value != nullptr)); }
    public:
        T value;
        non_null_lateinit() {

        }

        non_null_lateinit(T value): value(value) {
            checkForNull();
        }

        operator T() {
            checkForNull();
            return value;
        }
    };

    template<typename T>
    struct non_null: non_null_lateinit<T> {
        non_null(T value): non_null_lateinit<T>(value) {}
    };

}