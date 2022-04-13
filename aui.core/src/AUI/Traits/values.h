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
#include <functional>
#include <optional>
#include <AUI/Common/SharedPtrTypes.h>

namespace aui {
    /**
     * @brief Forbids copy of your class.
     *
     * @code{cpp}
     * class MyObject: public aui::noncopyable {
     * private:
     *     void* mResource;
     * }
     * ...
     * MyObject obj1;
     * MyObject obj2 = obj1; // error
     * MyObject obj3 = std::move(obj); // but this one is ok
     * @endcode
     */
    struct noncopyable {
        noncopyable() = default;
        noncopyable(const noncopyable&) = delete;
    };


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

        operator T() const noexcept {
            checkForNull();
            return value;
        }
        auto operator->() const {
            checkForNull();
            return &*value;
        }
    };

    template<typename T>
    struct non_null: non_null_lateinit<T> {
        non_null(T value): non_null_lateinit<T>(value) {}
    };

    /**
     * @brief Does not allow escaping, allowing to accept lvalue ref, rvalue ref, shared_ptr and etc without overhead
     *
     *        Promises that the contained object wouldn't be copied/moved outside of the function thus does not take
     *        responsibility of deleting the object. This allows to accept lvalue and rvalue references, pointers,
     *        unique_ptr and shared_ptr without ref counter modification.
     *
     *        Intended to use in function arguments.
     *
     *        Accepts lvalue ref, rvalue ref, ptr and shared_ptr. Does not accepts null.
     *
     * @tparam T undecorated type
     */
    template<typename T>
    struct no_escape {
    static_assert(!std::is_reference<T>::value, "use undecorated type (without reference)");
    static_assert(!std::is_pointer_v<T>, "use undecorated type (without pointer)");
    private:
        T* value;

    public:
        no_escape(T& value): value(&value) {}
        no_escape(T&& value): value(&value) {}
        no_escape(T* value): value(value) {}

        template<typename ConvertibleToT>
        no_escape(const _<ConvertibleToT>& value): value(&*value) {}

        template<typename ConvertibleToT>
        no_escape(const _unique<ConvertibleToT>& value): value(&*value) {}

        T* operator->() const {
            return value;
        }

        T& operator*() const {
            return *value;
        }
    };

    /**
     * A value that initializes when accessed for the first time.
     * @tparam T
     */
    template<typename T>
    struct lazy {
    private:
        mutable std::optional<T> value;
        std::function<T()> initializer;
    public:
        template<typename Factory, std::enable_if_t<std::is_invocable_r_v<T, Factory>, bool> = true>
        lazy(Factory&& initializer) noexcept : initializer(std::forward<Factory>(initializer)) {}

        lazy(const lazy<int>& other) noexcept: value(other.value), initializer(other.initializer) {}
        lazy(lazy<int>&& other) noexcept: value(std::move(other.value)), initializer(std::move(other.initializer)) {}

        T& get() {
            if (!value) {
                value = initializer();
                initializer = nullptr;
            }
            return *value;
        }
        const T& get() const {
            return const_cast<lazy<T>*>(this)->get();
        }

        operator T&() {
            return get();
        }
        operator const T&() const {
            return get();
        }

        T& operator*() {
            return get();
        }
        const T& operator*() const {
            return get();
        }

        T* operator->() {
            return &get();
        }
        T const * operator->() const {
            return &get();
        }
    };

    namespace promise {
        /**
         * Forbids copy of the wrapped value.
         * The caller can be sure his value wouldn't be copied.
         * @tparam T
         */
        template<typename T>
        class no_copy {
        private:
            T* value;

        public:
            no_copy(T& value): value(&value) {          // implicit initializer

            }
            operator T&() const {                      // implicit conversion
                return *value;
            }

            T& operator*() const {                     // std dereference
                return *value;
            }
            T* operator->() const {                    // allow pointer-style calls
                return value;
            }
        };
    }
}