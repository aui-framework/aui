/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <type_traits>
#include <AUI/Util/Assert.h>

class AObject;

#ifdef AUI_SHARED_PTR_FIND_INSTANCES
#include <AUI/api.h>
#include <AUI/Util/APimpl.h>
#include <map>
#include <set>
#include <mutex>
#include <AUI/Reflect/AClass.h>

class API_AUI_CORE AStacktrace;

namespace aui::impl::shared_ptr {
    struct InstancesDict {
        std::recursive_mutex sync;
        std::map<void*, std::set<void*>> map;
    };

    API_AUI_CORE InstancesDict& instances() noexcept;
    API_AUI_CORE void printAllInstancesOf(void* ptrToSharedPtr) noexcept;
}

#endif

template<typename T>
class AArc;

/**
 * Deprecated, use AArc or aui::Arc instead
 */
template<typename T>
using _ = AArc<T>;

/**
 * @brief An std::weak_ptr with AUI extensions.
 * @tparam T
 */
template<typename T>
struct _weak: public std::weak_ptr<T> {
private:
    using super = std::weak_ptr<T>;

public:
    using super::weak_ptr;

    _weak(const _weak<T>& v) noexcept: std::weak_ptr<T>(v) {}
    _weak(_weak<T>&& v) noexcept: std::weak_ptr<T>(std::move(v)) {}
    _weak(const std::weak_ptr<T>& v): std::weak_ptr<T>(v) {}
    _weak(std::weak_ptr<T>&& v) noexcept: std::weak_ptr<T>(std::move(v)) {}

    AArc<T> lock() const noexcept {
        return static_cast<AArc<T>>(super::lock());
    }

    _weak& operator=(const std::weak_ptr<T>& v) noexcept {
        super::weak_ptr::operator=(v);
        return *this;
    }

    _weak& operator=(std::weak_ptr<T>&& v) noexcept {
        super::weak_ptr::operator=(std::move(v));
        return *this;
    }

    _weak& operator=(const _weak<T>& v) noexcept {
        super::weak_ptr::operator=(v);
        return *this;
    }

    _weak& operator=(_weak<T>&& v) noexcept {
        super::weak_ptr::operator=(std::move(v));
        return *this;
    }

    _weak& operator=(const std::shared_ptr<T>& v) noexcept {
        super::weak_ptr::operator=(v);
        return *this;
    }

    _weak& operator=(std::shared_ptr<T>&& v) noexcept {
        super::weak_ptr::operator=(std::move(v));
        return *this;
    }

    _weak& operator=(const AArc<T>& v) noexcept {
        super::weak_ptr::operator=(v);
        return *this;
    }

    _weak& operator=(AArc<T>&& v) noexcept {
        super::weak_ptr::operator=(std::move(v));
        return *this;
    }
};

template<typename T, typename Deleter = std::default_delete<T>>
using _unique = std::unique_ptr<T, Deleter>;


namespace aui {

    struct ptr {
        /**
         * @brief Creates unique_ptr from raw pointer and a deleter.
         * @tparam T The type of the object to manage.
         * @tparam Deleter The type of the custom deleter function or functor.
         * @param ptr A raw pointer to the object that the unique pointer will manage.
         * @param deleter A custom deleter that will be used to destroy the managed object.
         * @return A unique pointer that takes ownership of the raw pointer using the specified deleter.
         * @details
         * Creates a unique pointer with a custom deleter.
         *
         * `unique_ptr` could not deduce T and Deleter by itself. Use this function to avoid this restriction.
         * By using this function, the lifetime of the pointer is delegated to std::unique_ptr. Specified Deleter will
         * free the wrapped pointer. The default deleter is std::default_delete. You may want to specialize
         * `std::default_delete<T>` struct to specify a default deleter for T, in this case you can omit the deleter
         * argument of this function.
         */
        template<typename T, typename Deleter = std::default_delete<T>>
        static _unique<T, Deleter> manage_unique(T* ptr, Deleter deleter = Deleter{}) {
            return { ptr, std::move(deleter) };
        }

        /**
         * @brief Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is
         * returned.
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static AArc<T> manage_shared(T* raw);

        /**
         * @brief Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is
         * returned, with a deleter functor.
         * @tparam T any type
         * @tparam Deleter object implementing <code>operator()(T*)</code>
         * @param raw raw pointer to manage memory of
         * @param deleter
         * @return shared pointer
         */
        template<typename T, typename Deleter>
        static AArc<T> manage_shared(T* raw, Deleter deleter);

        /**
         * @brief Creates fake shared pointer to <code>T* raw</code> with empty destructor, which does nothing. It's useful
         * when some function accepts a shared pointer but you have only a raw one.
         * @tparam T any type.
         * @param raw raw pointer to manage memory of.
         * @return shared pointer.
         */
        template<typename T>
        static AArc<T> fake_shared(T* raw);

        /**
         * @brief Downcasts `std::enable_shared_from_this<base class>` to the derived class `T`.
         * @tparam T class which derived from a class that implements `std::enable_shared_from_this`.
         * @param raw pointer to the `T` class.
         * @return Shared pointer.
         * @details
         * `std::enable_shared_from_this` provides a shared pointer to itself, however, if inheritance takes place, type
         * is lost, requiring manual downcasting.
         *
         * Downcasts `std::enable_shared_from_this<base class>` to the derived class `T` by the aliasing constructor of
         * the shared pointer. This eliminates the need of expensive downcasting, maintaining memory safety.
         */
        template <typename T>
        static AArc<T> shared_from_this(T* raw) {
            return AArc<T>(raw->shared_from_this(), raw);
        }

        /**
         * @brief Downcasts `std::enable_shared_from_this<base class>` to the derived class `T`.
         * @tparam T class which derived from a class that implements `std::enable_shared_from_this`.
         * @param raw pointer to the `T` class.
         * @return Weak pointer.
         * @details
         * `std::enable_shared_from_this` provides a shared pointer to itself, however, if inheritance takes place, type
         * is lost, requiring manual downcasting.
         *
         * Downcasts `std::enable_shared_from_this<base class>` to the derived class `T` by the aliasing constructor of
         * the shared pointer. This eliminates the need of expensive downcasting, maintaining memory safety.
         */
        template <typename T>
        static _weak<T> weak_from_this(T* raw) {
            // std::weak_ptr not having an aliasing constructor is clearly intentional rather than oversight --
            // although i dont understand reasons behind it
            return _weak<T>(shared_from_this(raw));
        }
    };
}


/**
 * @brief @brief An std::weak_ptr with AUI extensions.
 * @details
 * !!! note
 *
 *     Of course, it is not good tone to define a class with _ type but it significantly increases coding speed. Instead
 *     of writing every time std::shared_ptr you should write only the _ symbol.
 */
template<typename T>
class AArc : public std::shared_ptr<T>
{
    friend struct aui::ptr;
private:
    using super = std::shared_ptr<T>;

    AArc(T* raw, std::nullopt_t): std::shared_ptr<T>(raw) {

    }

#ifdef AUI_SHARED_PTR_FIND_INSTANCES
    friend API_AUI_CORE void aui::impl::shared_ptr::printAllInstancesOf(void* ptrToSharedPtr) noexcept;
    struct InstanceDescriber;

    static AStacktrace makeStacktrace();

    struct InstanceDescriber {
        aui::fast_pimpl<AStacktrace, sizeof(std::vector<int>) + 8, alignof(std::vector<int>)> stacktrace;
        AArc<T>* self;
        T* pointingTo;

        InstanceDescriber(AArc<T>* self): stacktrace(AArc<T>::makeStacktrace()), self(self), pointingTo(self->get()) {
            if (pointingTo == nullptr) return;
            std::unique_lock lock(aui::impl::shared_ptr::instances().sync);
            aui::impl::shared_ptr::instances().map[pointingTo].insert(self);
            //std::printf("[AUI_SHARED_PTR_FIND_INSTANCES] %s << %p\n", AClass<T>::name().toStdString().c_str(), pointingTo);
        }

        InstanceDescriber(const InstanceDescriber&) = delete;
        InstanceDescriber(InstanceDescriber&&) = delete;

        ~InstanceDescriber() {
            if (pointingTo == nullptr) return;
            auto& inst = aui::impl::shared_ptr::instances();
            std::unique_lock lock(inst.sync);
            if (auto mapIt = inst.map.find(pointingTo); mapIt != inst.map.end()) {
                auto setIt = mapIt->second.find(self);
                AUI_ASSERT(setIt != mapIt->second.end());
                mapIt->second.erase(setIt);
                if (mapIt->second.empty()) {
                    //inst.map.erase(it);
                }
            }
            //std::printf("[AUI_SHARED_PTR_FIND_INSTANCES] %s >> %p\n", AClass<T>::name().toStdString().c_str(), pointingTo);
        }
    } mInstanceDescriber = this;

#endif

public:
    using stored_t = T;


#ifdef AUI_SHARED_PTR_FIND_INSTANCES
    void printAllInstances() {
        aui::impl::shared_ptr::printAllInstancesOf(this);
    }
    void printAllInstances() const {
        const_cast<AArc<T>&>(*this).printAllInstances();
    }
#endif

    using std::shared_ptr<T>::shared_ptr;

    AArc(const std::shared_ptr<T>& v): std::shared_ptr<T>(v) {}
    AArc(std::shared_ptr<T>&& v) noexcept: std::shared_ptr<T>(std::move(v)) {}
    AArc(const AArc& v): std::shared_ptr<T>(v) {}
    AArc(AArc&& v) noexcept: std::shared_ptr<T>(std::move(v)) {}
    AArc(const std::weak_ptr<T>& v): std::shared_ptr<T>(v) {}
    AArc(const _weak<T>& v): std::shared_ptr<T>(v) {}

    AArc& operator=(const AArc& rhs) noexcept {
        std::shared_ptr<T>::operator=(rhs);
        return *this;
    }

    AArc& operator=(AArc&& rhs) noexcept {
        std::shared_ptr<T>::operator=(std::forward<AArc>(rhs));
        return *this;
    }

    /**
     * <p>Trap constructor</p>
     * <p>In order to make shared pointer from the raw one, please explicitly specify how do you want manage memory by
     * using either <code>aui::ptr::manage</code> or <code>aui::ptr::fake</code>.
     */
    AArc(T* v) = delete;

    /**
     * @return weak reference
     */
    [[nodiscard]]
    _weak<T> weak() const {
        return _weak<T>(*this);
    }

    template<typename SignalField, typename Object, typename Function>
    inline AArc<T>& connect(SignalField signalField, Object object, Function&& function);

    template<typename SignalField, typename Function>
    inline AArc<T>& connect(SignalField signalField, Function&& function);


    template <typename Functor>
    const AArc<T>& operator^(Functor&& functor) const {
        functor(*this);
        return *this;
    }

    /**
     * @brief Dereferences the stored pointer.
     * @details
     * On a debug build, throws an assertion failure if the stored pointer is `nullptr`, otherwise behaviour is
     * undefined.
     */
    [[nodiscard]]
    std::add_lvalue_reference_t<T> value() const noexcept {
#if AUI_DEBUG
        AUI_ASSERTX(super::get() != nullptr, "an attempt to dereference a null pointer");
#endif
        return *super::get();
    }

    /**
     * @brief Dereferences the stored pointer.
     * @details
     * On a debug build, throws an assertion failure if the stored pointer is `nullptr`, otherwise behaviour is
     * undefined.
     */
    [[nodiscard]]
    std::add_lvalue_reference_t<T> operator*() const noexcept {
        return value();
    }

    /**
     * @brief Dereferences the stored pointer.
     * @details
     * On a debug build, throws an assertion failure if the stored pointer is `nullptr`, otherwise behaviour is
     * undefined.
     */
    [[nodiscard]]
    std::add_pointer_t<T> operator->() const noexcept {
        return &value();
    }

    // forward ranged-for loops
    auto begin() const requires requires(T& t) { t.begin(); } {
        return super::operator->()->begin();
    }
    auto end() const requires requires(T& t) { t.end(); } {
        return super::operator->()->end();
    }
    auto begin() requires requires(T& t) { t.begin(); } {
        return super::operator->()->begin();
    }
    auto end() requires requires(T& t) { t.end(); } {
        return super::operator->()->end();
    }

    // operators

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) << std::forward<Arg>(r); }
    const AArc<T>& operator<<(Arg&& value) const {
        (*super::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    AArc<T>& operator<<(Arg&& value) {
        (*super::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename... Arg>
    auto operator()(Arg&&... value) const requires std::is_invocable_v<T, Arg...> {
        return (*super::get())(std::forward<Arg>(value)...);
    }

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) + std::forward<Arg>(r); }
    const AArc<T>& operator+(Arg&& value) const {
        (*super::get()) + std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) & std::forward<Arg>(r); }
    const AArc<T>& operator&(Arg&& value) const {
        (*super::get()) & std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) | std::forward<Arg>(r); }
    const AArc<T>& operator|(Arg&& value) const {
        (*super::get()) | std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    AArc<T>& operator+(Arg&& value) {
        (*super::get()) + std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    const AArc<T>& operator*(Arg&& value) {
        (*super::get()) * std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) - std::forward<Arg>(r); }
    const AArc<T>& operator-(Arg&& value) const {
        (*super::get()) - std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    AArc<T>& operator-(Arg&& value) {
        (*super::get()) - std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    requires requires (T&& l, Arg&& r) { std::forward<T>(l) >> std::forward<Arg>(r); }
    const AArc<T>& operator>>(Arg&& value) const {
        (*super::get()) >> std::forward<Arg>(value);
        return *this;
    }

    template<typename...Args>
    AArc<T>& operator()(const Args&... value) {
        (*super::get())(value...);
        return *this;
    }
    template<typename...Args>
    auto operator()(Args&&... value) {
        return (*super::get())(std::forward<Args>(value)...);
    }
};


namespace aui {
    template<typename T>
    AArc<T> ptr::manage_shared(T* raw) {
        return AArc<T>(raw, std::nullopt);
    }

    template<typename T>
    AArc<T> ptr::fake_shared(T* raw) {
        return AArc<T>(std::shared_ptr<void>{}, raw);
    }

    template<typename T, typename Deleter>
    AArc<T> ptr::manage_shared(T* raw, Deleter deleter) {
        return AArc<T>(raw, deleter);
    }
}

template<typename TO, typename FROM>
inline TO* _cast(const _unique<FROM>& object)
{
    return dynamic_cast<TO*>(object.get());
}

template<typename TO, typename FROM>
inline AArc<TO> _cast(const AArc<FROM>& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(object);
}

template<typename TO, typename FROM>
inline AArc<TO> _cast(AArc<FROM>&& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(std::move(object));
}

template<typename TO, typename FROM>
inline AArc<TO> _cast(const std::shared_ptr<FROM>& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(object);
}


template<typename TO, typename FROM>
inline AArc<TO> _cast(std::shared_ptr<FROM>&& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(std::move(object));
}



/**
 * @brief nullsafe call (see examples).
 * @ingroup useful_macros
 *
 * <table>
 *   <tr>
 *     <td>without</td>
 *     <td>with</td>
 *   </tr>
 *   <tr>
 *     <td>
 *       ```cpp
 *       if (getAnimator()) getAnimator()->postRender(this);
 *       ```
 *     </td>
 *     <td>
 *       ```cpp
 *       AUI_NULLSAFE(getAnimator())->postRender(this);
 *       ```
 *     </td>
 *   </tr>
 * </table>
 *
 * which is shorter, avoids code duplication and calls <code>getAnimator()</code> only once because <code>AUI_NULLSAFE</code> expands to:
 *
 * ```cpp
 * if (auto& _tmp = (getAnimator())) _tmp->postRender(this);
 * ```
 *
 * Since `AUI_NULLSAFE` is a macro that expands to `if`, you can use `else` keyword:
 *
 * ```cpp
 * AUI_NULLSAFE(getWindow())->flagRedraw(); else ALogger::info("Window is null!");
 * ```
 *
 * and even combine multiple `AUI_NULLSAFE` statements:
 *
 * ```cpp
 * AUI_NULLSAFE(getWindow())->flagRedraw(); else AUI_NULLSAFE(AWindow::current())->flagRedraw();
 * ```
 */
#define AUI_NULLSAFE(s) if(decltype(auto) _tmp = (s))_tmp