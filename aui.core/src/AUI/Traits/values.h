/*
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
#include <AUI/Thread/AMutex.h>

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
            return AnyType(mValue);
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

        no_escape(const _<T>& value): value(&*value) {}
        no_escape(const _unique<T>& value): value(&*value) {}

        template<typename DerivedFromT, std::enable_if_t<std::is_base_of_v<T, DerivedFromT> && !std::is_same_v<DerivedFromT, T>, bool> = true>
        no_escape(const _<DerivedFromT>& value): value(&*value) {}

        template<typename DerivedFromT, std::enable_if_t<std::is_base_of_v<T, DerivedFromT> && !std::is_same_v<DerivedFromT, T>, bool> = true>
        no_escape(const _unique<DerivedFromT>& value): value(&*value) {}

        T* operator->() const {
            return value;
        }

        T& operator*() const {
            return *value;
        }
    };

    /**
     * @brief A value that initializes when accessed for the first time.
     * @tparam T
     */
    template<typename T = void>
    struct lazy {
    private:
        mutable std::optional<T> value;
        std::function<T()> initializer;
    public:
        template<typename Factory, std::enable_if_t<std::is_invocable_r_v<T, Factory>, bool> = true>
        lazy(Factory&& initializer) noexcept : initializer(std::forward<Factory>(initializer)) {}

        lazy(const lazy<T>& other) noexcept: value(other.value), initializer(other.initializer) {}
        lazy(lazy<T>&& other) noexcept: value(std::move(other.value)), initializer(std::move(other.initializer)) {}

        T& get() {
            if (!value) {
                value = initializer();
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

        lazy<T>& operator=(T&& t) {
            value = std::forward<T>(t);
            return *this;
        }
        lazy<T>& operator=(const T& t) {
            value = t;
            return *this;
        }

        void reset() {
            value.reset();
        }

        [[nodiscard]]
        bool has_value() const noexcept {
            return value.has_value();
        }
    };

    /**
     * @brief A value that initializes when accessed for the first time.
     * @tparam T
     */
    template<>
    struct lazy<void> {
    private:
        mutable bool value = false;
        std::function<void()> initializer;
    public:
        template<typename Factory, std::enable_if_t<std::is_invocable_r_v<void, Factory>, bool> = true>
        lazy(Factory&& initializer) noexcept : initializer(std::forward<Factory>(initializer)) {}

        lazy(const lazy<void>& other) noexcept: value(other.value), initializer(other.initializer) {}
        lazy(lazy<void>&& other) noexcept: value(other.value), initializer(std::move(other.initializer)) {}

        void get() {
            if (!value) {
                value = true;
                initializer();
            }
        }
        void get() const {
            return const_cast<lazy<void>*>(this)->get();
        }

        void operator*() {
            return get();
        }
        const void operator*() const {
            return get();
        }


        void reset() {
            value = false;
        }

        [[nodiscard]]
        bool has_value() const noexcept {
            return value;
        }
    };

    /**
     * @brief A value that initializes when accessed for the first time.
     * Unlike <code>aui::lazy</code>, internal logic of <code>aui::atomic_lazy</code> is threadsafe.
     * @tparam T
     */
    template<typename T>
    struct atomic_lazy {
    private:
        mutable AMutex sync;
        mutable std::optional<T> value;
        std::function<T()> initializer;
    public:
        template<typename Factory, std::enable_if_t<std::is_invocable_r_v<T, Factory>, bool> = true>
        atomic_lazy(Factory&& initializer) : initializer(std::forward<Factory>(initializer)) {}

        atomic_lazy(const atomic_lazy<T>& other) {
            std::unique_lock lock(other.sync);
            value = other.value;
            initializer = other.initializer;
        }
        atomic_lazy(atomic_lazy<T>&& other) noexcept {
            std::unique_lock lock(other.sync);
            value = std::move(other.value);
            initializer = std::move(other.initializer);
        }

        T& get() {
            if (!value) {
                std::unique_lock lock(sync);
                if (!value) {
                    value = initializer();
                }
            }
            return *value;
        }
        const T& get() const {
            return const_cast<atomic_lazy<T>*>(this)->get();
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

        atomic_lazy<T>& operator=(T&& t) {
            std::unique_lock lock(sync);
            value = std::forward<T>(t);
            return *this;
        }

        atomic_lazy<T>& operator=(const T& t) {
            std::unique_lock lock(sync);
            value = t;
            return *this;
        }

        void reset() {
            std::unique_lock lock(sync);
            value.reset();
        }

        [[nodiscard]]
        bool has_value() const noexcept {
            std::unique_lock lock(sync);
            return value.has_value();
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