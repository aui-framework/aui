/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
class _;

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

    _<T> lock() const noexcept {
        return static_cast<_<T>>(super::lock());
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

    _weak& operator=(const _<T>& v) noexcept {
        super::weak_ptr::operator=(v);
        return *this;
    }

    _weak& operator=(_<T>&& v) noexcept {
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
         * @details
         * `unique_ptr` could not deduce T and Deleter by itself. Use this function to avoid this restriction.
         * By using this function, lifetime of the pointer is delegated to std::unique_ptr. The wrapped pointer will
         * be freed by specified Deleter. Default deleter is std::default_delete. You may want to specialize
         * `std::default_delete<T>` struct in order to specify default deleter for T, in this case you can omit deleter
         * argument of this function.
         */
        template<typename T, typename Deleter = std::default_delete<T>>
        static _unique<T, Deleter> make_unique_with_deleter(T* ptr, Deleter deleter = Deleter{}) {
            return { ptr, std::move(deleter) };
        }

        /**
         * @brief Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static _<T> manage(T* raw);

        /**
         * @brief Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @tparam Deleter object implementing <code>operator()(T*)</code>
         * @param raw raw pointer to manage memory of
         * @param deleter
         * @return shared pointer
         */
        template<typename T, typename Deleter>
        static _<T> manage(T* raw, Deleter deleter);

        /**
         * @brief Delegates memory management of the raw pointer <code>T* raw</code> to the unique pointer, which is returned
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return unique pointer
         */
        template<typename T>
        static _unique<T> unique(T* raw);

        /**
         * @brief Creates fake shared pointer to <code>T* raw</code> with empty destructor, which does nothing. It's useful
         * when some function accept shared pointer but you have only raw one.
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static _<T> fake(T* raw);
    };
}


/**
 * @brief @brief An std::weak_ptr with AUI extensions.
 * @note  Of course, it is not good tone to define a class with _ type but it significantly increases coding speed.
 *        Instead of writing every time std::shared_ptr you should write only the _ symbol.
 */
template<typename T>
class _ : public std::shared_ptr<T>
{
    friend struct aui::ptr;
private:
    using super = std::shared_ptr<T>;

    _(T* raw, std::nullopt_t): std::shared_ptr<T>(raw) {

    }

#ifdef AUI_SHARED_PTR_FIND_INSTANCES
    friend API_AUI_CORE void aui::impl::shared_ptr::printAllInstancesOf(void* ptrToSharedPtr) noexcept;
    struct InstanceDescriber;

    static AStacktrace makeStacktrace();

    struct InstanceDescriber {
        aui::fast_pimpl<AStacktrace, sizeof(std::vector<int>) + 8, alignof(std::vector<int>)> stacktrace;
        _<T>* self;
        T* pointingTo;

        InstanceDescriber(_<T>* self): stacktrace(_<T>::makeStacktrace()), self(self), pointingTo(self->get()) {
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
        const_cast<_<T>&>(*this).printAllInstances();
    }
#endif

    using std::shared_ptr<T>::shared_ptr;

    _(const std::shared_ptr<T>& v): std::shared_ptr<T>(v) {}
    _(std::shared_ptr<T>&& v) noexcept: std::shared_ptr<T>(std::move(v)) {}
    _(const _& v): std::shared_ptr<T>(v) {}
    _(_&& v) noexcept: std::shared_ptr<T>(std::move(v)) {}
    _(const std::weak_ptr<T>& v): std::shared_ptr<T>(v) {}
    _(const _weak<T>& v): std::shared_ptr<T>(v) {}

    _& operator=(const _& rhs) noexcept {
        std::shared_ptr<T>::operator=(rhs);
        return *this;
    }

    _& operator=(_&& rhs) noexcept {
        std::shared_ptr<T>::operator=(std::forward<_>(rhs));
        return *this;
    }

    /**
     * <p>Trap constructor</p>
     * <p>In order to make shared pointer from the raw one, please explicitly specify how do you want manage memory by
     * using either <code>aui::ptr::manage</code> or <code>aui::ptr::fake</code>.
     */
    _(T* v) = delete;

    /**
     * @return weak reference
     */
    [[nodiscard]]
    _weak<T> weak() const {
        return _weak<T>(*this);
    }

    template<typename SignalField, typename Object, typename Function>
    inline _<T>& connect(SignalField signalField, Object object, Function&& function);

    template<typename SignalField, typename Function>
    inline _<T>& connect(SignalField signalField, Function&& function);


    template <typename Functor>
    inline _<T>& operator^(Functor&& functor) {
        functor(*this);
        return *this;
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
    const _<T>& operator<<(Arg&& value) const {
        (*super::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    _<T>& operator<<(Arg&& value) {
        (*super::get()) << std::forward<Arg>(value);
        return *this;
    }

    template<typename... Arg>
    auto operator()(Arg&&... value) const requires std::is_invocable_v<T, Arg...> {
        return (*super::get())(std::forward<Arg>(value)...);
    }

    template<typename Arg>
    const _<T>& operator+(Arg&& value) const {
        (*super::get()) + std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    _<T>& operator+(Arg&& value) {
        (*super::get()) + std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    const _<T>& operator*(Arg&& value) {
        (*super::get()) * std::forward<Arg>(value);
        return *this;
    }

    [[nodiscard]]
    std::add_lvalue_reference_t<T> operator*() const noexcept {
        return super::operator*();
    }

    template<typename Arg>
    const _<T>& operator-(Arg&& value) const {
        (*super::get()) - std::forward<Arg>(value);
        return *this;
    }

    template<typename Arg>
    _<T>& operator-(Arg&& value) {
        (*super::get()) - std::forward<Arg>(value);
        return *this;
    }
    template<typename Arg>
    _<T>& operator>>(Arg&& value) {
        (*super::get()) >> std::forward<Arg>(value);
        return *this;
    }

    template<typename...Args>
    _<T>& operator()(const Args&... value) {
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
    _<T> ptr::manage(T* raw) {
        return _<T>(raw, std::nullopt);
    }

    template<typename T>
    _<T> ptr::fake(T* raw) {
        return _<T>(std::shared_ptr<void>{}, raw);
    }

    template<typename T, typename Deleter>
    _<T> ptr::manage(T* raw, Deleter deleter) {
        return _<T>(raw, deleter);
    }
    template<typename T>
    _unique<T> ptr::unique(T* raw) {
        return _unique<T>(raw);
    }
}

template<typename TO, typename FROM>
inline TO* _cast(const _unique<FROM>& object)
{
    return dynamic_cast<TO*>(object.get());
}

template<typename TO, typename FROM>
inline _<TO> _cast(const _<FROM>& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(object);
}

template<typename TO, typename FROM>
inline _<TO> _cast(_<FROM>&& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(std::move(object));
}

template<typename TO, typename FROM>
inline _<TO> _cast(const std::shared_ptr<FROM>& object)
{
    return std::dynamic_pointer_cast<TO, FROM>(object);
}


template<typename TO, typename FROM>
inline _<TO> _cast(std::shared_ptr<FROM>&& object)
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
 *       @code{cpp}
 *       if (getAnimator()) getAnimator()->postRender(this);
 *       @endcode
 *     </td>
 *     <td>
 *       @code{cpp}
 *       AUI_NULLSAFE(getAnimator())->postRender(this);
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 *
 * which is shorter, avoids code duplication and calls <code>getAnimator()</code> only once because <code>AUI_NULLSAFE</code> expands to:
 *
 * @code{cpp}
 * if (auto& _tmp = (getAnimator())) _tmp->postRender(this);
 * @endcode
 *
 * Since `AUI_NULLSAFE` is a macro that expands to `if`, you can use `else` keyword:
 *
 * @code{cpp}
 * AUI_NULLSAFE(getWindow())->flagRedraw(); else ALogger::info("Window is null!");
 * @endcode
 *
 * and even combine multiple `AUI_NULLSAFE` statements:
 *
 * @code{cpp}
 * AUI_NULLSAFE(getWindow())->flagRedraw(); else AUI_NULLSAFE(AWindow::current())->flagRedraw();
 * @endcode
 */
#define AUI_NULLSAFE(s) if(decltype(auto) _tmp = (s))_tmp