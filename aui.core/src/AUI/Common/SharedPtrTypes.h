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

#include <memory>
#include <functional>
#include <optional>

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
        std::mutex sync;
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

    _weak(const std::weak_ptr<T>& v): std::weak_ptr<T>(v) {}
    _weak(std::weak_ptr<T>&& v): std::weak_ptr<T>(std::forward<std::weak_ptr<T>>(v)) {}

    _<T> lock() const noexcept {
        return static_cast<_<T>>(super::lock());
    }
};

template<typename T>
using _unique = std::unique_ptr<T>;


namespace aui {
    struct ptr {
        /**
         * Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return shared pointer
         */
        template<typename T>
        static _<T> manage(T* raw);

        /**
         * Delegates memory management of the raw pointer <code>T* raw</code> to the shared pointer, which is returned
         * @tparam T any type
         * @tparam Deleter object implementing <code>operator()(T*)</code>
         * @param raw raw pointer to manage memory of
         * @param deleter
         * @return shared pointer
         */
        template<typename T, typename Deleter>
        static _<T> manage(T* raw, Deleter deleter);
        /**
         * Delegates memory management of the raw pointer <code>T* raw</code> to the unique pointer, which is returned
         * @tparam T any type
         * @param raw raw pointer to manage memory of
         * @return unique pointer
         */
        template<typename T>
        static _unique<T> unique(T* raw);

        /**
         * Creates fake shared pointer to <code>T* raw</code> with empty destructor, which does nothing. It's useful
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
                assert(setIt != mapIt->second.end());
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

    class SafeCallWrapper
    {
    private:
        _<T>& mPtr;

    public:
        SafeCallWrapper(_<T>& ptr)
                : mPtr(ptr)
        {
        }

        template<typename MemberFunction, typename... Args>
        SafeCallWrapper& operator()(MemberFunction memberFunction, Args&& ... args) {
            if (mPtr)
                (mPtr.get()->*memberFunction)(std::forward<Args>(args)...);
            return *this;
        }
    };

    using std::shared_ptr<T>::shared_ptr;

    _(const std::shared_ptr<T>& v): std::shared_ptr<T>(v) {}
    _(std::shared_ptr<T>&& v): std::shared_ptr<T>(std::forward<std::shared_ptr<T>>(v)) {}
    _(const _& v): std::shared_ptr<T>(v) {}
    _(_&& v): std::shared_ptr<T>(std::forward<_>(v)) {}
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
    _weak<T> weak() {
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

    /**
     * @brief Guarantees that further builder calls will be executed if and only if this pointer
     *        not equal to null.
     * @return safe builder
     * @deprecated use nullsafe() instead
     */
    [[deprecated]]
    inline auto safe()
    {
        return SafeCallWrapper(*this);
    }

    // forward ranged-for loops
    auto begin() const {
        return super::operator->()->begin();
    }
    auto end() const {
        return super::operator->()->end();
    }
    auto begin() {
        return super::operator->()->begin();
    }
    auto end() {
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
        return _<T>(raw, [](T*) {});
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
inline _<TO> _cast(_<FROM> object)
{
    return std::dynamic_pointer_cast<TO, FROM>(object);
}


/**
 * @brief Nullsafe call (see examples).
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
 *       nullsafe(getAnimator())->postRender(this);
 *       @endcode
 *     </td>
 *   </tr>
 * </table>
 *
 * which is shorter, avoids code duplication and calls <code>getAnimator()</code> only once because <code>nullsafe</code> expands to:
 *
 * @code{cpp}
 * if (auto& _tmp = (getAnimator())) _tmp->postRender(this);
 * @endcode
 *
 * Since `nullsafe` is a macro that expands to `if`, you can use `else` keyword:
 *
 * @code{cpp}
 * nullsafe(getWindow())->flagRedraw(); else ALogger::info("Window is null!");
 * @endcode
 *
 * and even combine multiple `nullsafe` statements:
 *
 * @code{cpp}
 * nullsafe(getWindow())->flagRedraw(); else nullsafe(AWindow::current())->flagRedraw();
 * @endcode
 */
#define nullsafe(s) if(decltype(auto) _tmp = (s))_tmp