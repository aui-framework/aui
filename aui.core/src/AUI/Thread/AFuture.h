// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include "AConditionVariable.h"
#include "AMutex.h"
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>
#include <AUI/Reflect/AReflect.h>

class AThreadPool;


class AInvocationTargetException: public AException {

public:
    AInvocationTargetException(const AString& message = {}): AException(message, std::current_exception(), AStacktrace::capture(3)) {}


    ~AInvocationTargetException() noexcept override = default;
};


/**
 * Controls <code>AFuture::wait</code> behaviour.
 * @see AFuture::wait
 */
AUI_ENUM_FLAG(AFutureWait) {
    DEFAULT = 0b01,
    ASYNC_ONLY = 0b00
};


namespace aui::impl::future {
    /**
     * This class calls cancel() and wait() methods of AFuture::Inner BEFORE AFuture::Inner destruction in order to keep
     * alive the weak reference created in AThreadPool::operator<<.
     */
    template<typename Inner>
    struct CancellationWrapper {

        explicit CancellationWrapper(_unique<Inner> wrapped) : wrapped(std::move(wrapped)) {}

        ~CancellationWrapper() {
            wrapped->cancel();
            wrapped->waitForTask();
        }

        [[nodiscard]]
        const _unique<Inner>& ptr() noexcept {
            return wrapped;
        }

        Inner* operator->() const noexcept {
            return wrapped.get();
        }

    private:
        _unique<Inner> wrapped;
    };

    template<typename T>
    struct OnSuccessCallback {
        using type = std::function<void(const T& value)>;
    };

    template<>
    struct OnSuccessCallback<void> {
        using type = std::function<void()>;
    };
    template<typename Value = void>
    class Future
    {
    public:
        static constexpr bool isVoid = std::is_same_v<void, Value>;
        using TaskCallback = std::function<Value()>;

        using OnSuccessCallback = typename OnSuccessCallback<Value>::type;

        struct Inner {
            bool interrupted = false;
            /**
             * When isVoid = true, bool is a flag whether "void value" supplied or not
             */
            std::conditional_t<isVoid, bool, AOptional<Value>> value;
            AOptional<AInvocationTargetException> exception;
            AMutex mutex;
            AConditionVariable cv;
            TaskCallback task;
            OnSuccessCallback onSuccess;
            std::function<void(const AException& exception)> onError;
            _<AAbstractThread> thread;
            bool cancelled = false;

            explicit Inner(std::function<Value()> task) noexcept: task(std::move(task)) {
                if constexpr(isVoid) {
                    value = false;
                }
            }

            void waitForTask() noexcept {
                std::unique_lock lock(mutex);
                bool rethrowInterrupted = false;
                while ((thread) && !hasResult() && !cancelled) {
                    try {
                        cv.wait(lock);
                    } catch (const AThread::Interrupted&) {
                        rethrowInterrupted = true;
                    }
                }
                if (rethrowInterrupted) {
                    AThread::current()->interrupt();
                }
            }

            [[nodiscard]]
            bool isWaitNeeded() noexcept {
                return (thread || !cancelled) && !hasResult();
            }

            [[nodiscard]]
            bool hasResult() const noexcept {
                return value || exception || interrupted;
            }

            [[nodiscard]]
            bool hasValue() const noexcept {
                return bool(value);
            }

            bool setThread(_<AAbstractThread> thr) noexcept {
                std::unique_lock lock(mutex);
                if (cancelled) return true;
                if (thread) return true;
                thread = std::move(thr);
                return false;
            }

            void wait(const _weak<CancellationWrapper<Inner>>& innerWeak, AFutureWait flags = AFutureWait::DEFAULT) noexcept {
                std::unique_lock lock(mutex);
                try {
                    if ((thread || !cancelled) && !hasResult() && int(flags) & int(AFutureWait::DEFAULT) && task) {
                        // task is not have picked up by the threadpool; execute it here
                        lock.unlock();
                        if (tryExecute(innerWeak)) {
                            return;
                        }
                        lock.lock();
                    }
                    while ((thread || !cancelled) && !hasResult()) {
                        cv.wait(lock);
                    }
                } catch (const AThread::Interrupted& e) {
                    e.needRethrow();
                }
            }

            void cancel() noexcept {
                std::unique_lock lock(mutex);
                if (!cancelled) {
                    cancelled = true;
                    if (thread && !hasResult()) {
                        thread->interrupt();
                    }
                }
            }

            /**
             * @brief Executes the task stored in the future.
             * Using weak_ptr to internal object in order to make possible Future cancellation by it's destruction.
             * @param innerWeak self weak_ptr
             * @return true, then task is successfully executed and supplied result.
             */
            bool tryExecute(const _weak<CancellationWrapper<Inner>>& innerWeak) {
                /*
                 * We should assume that this == nullptr or invalid.
                 * We can assert that this pointer is safe only if we hold at least one shared_ptr.
                 * It allows callers to pass a weak_ptr in any state.
                 */
                if (auto innerCancellation = innerWeak.lock()) {
                    if (value) return false;
                    auto& inner = innerCancellation->ptr();
                    if (inner->setThread(AThread::current())) return false;
                    try {
                        if (task == nullptr) { // task is executed in wait() function
                            return false;
                        }
                        std::unique_lock lock(mutex);
                        if (task == nullptr) { // task is executed in wait() function
                            return false;
                        }
                        auto func = std::move(task);
                        assert(bool(func));
                        lock.unlock();
                        innerCancellation = nullptr;
                        if constexpr(isVoid) {
                            func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                lock.lock();
                                value = true;
                                cv.notify_all();
                                notifyOnSuccessCallback();

                                (void)sharedPtrLock; // sharedPtrLock is *used*
                                lock.unlock(); // unlock earlier because destruction of shared_ptr may cause deadlock
                            }
                        } else {
                            auto result = func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                lock.lock();
                                value = std::move(result);
                                cv.notify_all();
                                notifyOnSuccessCallback();

                                (void)sharedPtrLock; // sharedPtrLock is *used*
                                lock.unlock(); // unlock earlier because destruction of shared_ptr may cause deadlock
                            }
                        }
                    } catch (const AException&) {
                        if (auto sharedPtrLock = innerWeak.lock()) {
                            inner->reportException();
                        }
                        return false;
                    } catch (...) {
                        if (auto sharedPtrLock = innerWeak.lock()) {
                            inner->reportInterrupted();
                        }
                        throw;
                    }
                }
                return true;
            }

            void reportInterrupted() noexcept {
                std::unique_lock lock(mutex);
                interrupted = true;
                cv.notify_all();
            }

            void reportException() noexcept {
                std::unique_lock lock(mutex);
                exception.emplace();
                cv.notify_all();
                AUI_NULLSAFE(onError)(*exception);
            }


            void notifyOnSuccessCallback() {
                if (value && onSuccess) {
                    if constexpr(isVoid) {
                        onSuccess();
                    } else {
                        onSuccess(*value);
                    }
                    onSuccess = nullptr;
                }
            }
        };

    protected:
        _<CancellationWrapper<Inner>> mInner;


    public:
        /**
         * @param task a callback which will be executed by Future::Inner::tryExecute. Can be null. If null, the result
         *        should be provided by AFuture::supplyResult function.
         */
        Future(TaskCallback task = nullptr) noexcept: mInner(_new<CancellationWrapper<Inner>>((_unique<Inner>)(new Inner(std::move(task))))) {}

        [[nodiscard]]
        const _<CancellationWrapper<Inner>>& inner() const noexcept {
            return mInner;
        }

        /**
         * @return true if call to wait() function would cause thread block.
         */
        [[nodiscard]]
        bool isWaitNeeded() const noexcept {
            return (*mInner)->isWaitNeeded();
        }

        /**
         * @return true if the value or exception or interruption received.
         */
        [[nodiscard]]
        bool hasResult() const noexcept {
            return (*mInner)->hasResult();
        }

        /**
         * @return true if the value can be obtained without waiting.
         */
        [[nodiscard]]
        bool hasValue() const noexcept {
            return (*mInner)->hasValue();
        }

        void reportException() noexcept {
            (*mInner)->reportException();
        }

        template<typename Callback>
        void onSuccess(Callback&& callback) const noexcept {
            std::unique_lock lock((*mInner)->mutex);
            if constexpr(isVoid) {
                if ((*mInner)->onSuccess) {
                    (*mInner)->onSuccess = [prev = std::move((*mInner)->onSuccess),
                            callback = std::forward<Callback>(callback)]() mutable {
                        prev();
                        callback();
                    };
                } else {
                    (*mInner)->onSuccess = [callback = std::forward<Callback>(callback)]() mutable {
                        callback();
                    };
                }
            } else {
                if ((*mInner)->onSuccess) {
                    (*mInner)->onSuccess = [prev = std::move((*mInner)->onSuccess),
                            callback = std::forward<Callback>(callback)](const Value& v) mutable {
                        prev(v);
                        callback(v);
                    };
                } else {
                    (*mInner)->onSuccess = [callback = std::forward<Callback>(callback)](
                            const Value& v) mutable {
                        callback(v);
                    };
                }
            }
            (*mInner)->notifyOnSuccessCallback();
        }

        template<typename Callback>
        void onError(Callback&& callback) const noexcept {
            std::unique_lock lock((*mInner)->mutex);

            if ((*mInner)->onError) {
                (*mInner)->onError = [prev = std::move((*mInner)->onError),
                        callback = std::forward<Callback>(callback)](const AException& v) {
                    prev(v);
                    callback(v);
                };
            } else {
                (*mInner)->onError = [callback = std::forward<Callback>(callback)](
                        const AException& v) {
                    callback(v);
                };
            }
        }


        /**
         * @brief Cancels the AFuture's task.
         * @details
         * When cancel() is called, there are 3 possible cases:
         * 1. AFuture's task is not taken by AThreadPool. The task is simply removed from the AThreadPool queue.
         * 2. AFuture's task is being executed. The task's thread is
         * @ref AAbstractThread::interrupt() "requested for interrupt".
         * 3. AFuture's task is already completed. cancel() does nothing.
         */
        void cancel() noexcept {
            (*mInner)->cancel();
        }

        void reportInterrupted() {
            (*mInner)->reportInterrupted();
        }

        /**
         * @brief Sleeps if the supplyResult is not currently available.
         * @note The task will be executed inside wait() function if the threadpool have not taken the task to execute
         *       yet. This behaviour can be disabled by <code>AFutureWait::ASYNC_ONLY</code> flag.
         */
        void wait(AFutureWait flags = AFutureWait::DEFAULT) noexcept {
            (*mInner)->wait(mInner, flags);
        }

        /**
         * @brief Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) get(AFutureWait flags = AFutureWait::DEFAULT) {
            AThread::interruptionPoint();
            (*mInner)->wait(mInner, flags);
            AThread::interruptionPoint();
            if ((*mInner)->exception) {
                throw *(*mInner)->exception;
            }
            if ((*mInner)->interrupted) {
                throw AInvocationTargetException("Future execution interrupted");
            }
            if constexpr(!isVoid) {
                return *(*mInner)->value;
            }
        }

        /**
         * @brief Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) operator*() {
            return get();
        }

        /**
         * @brief Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        Value* operator->() {
            return &operator*();
        }

        /**
         * @brief Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        Value const * operator->() const {
            return &operator*();
        }

        /**
         * @brief Returns the task result from the another thread. Sleeps if the task result is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        decltype(auto) operator*() const {
            return **const_cast<Future*>(this);
        }

    };

}


/**
 * @brief Represents the result of an asynchronous operation.
 * @ingroup core
 * @tparam T result type (void is default)
 * @details
 * AFuture is returned by @ref async keyword:
 *
 * @code{cpp}
 * AFuture<int> theFuture = async {
 *   AThread::sleep(1000); // long operation
 *   return 123;
 * };
 * cout << *theFuture; // 123
 * @endcode
 *
 * However, it can be default-constructed and the result can be supplied manually with the supplyResult() function:
 *
 * @code{cpp}
 * AFuture<int> theFuture;
 * AThread t([=] {
 *   AThread::sleep(1000); // long operation
 *   theFuture.supplyResult(123);
 * });
 * t.start();
 * cout << *theFuture; // 123
 * @endcode
 *
 * AFuture provides a set of functions to manage the process execution: cancel(), wait(), hasResult(), hasValue().
 *
 * AFuture is a shared_ptr-based wrapper so it can be easily copied, pointing to the same task.
 *
 * If all AFutures of the task are destroyed, the task is cancelled. If the task is executing when cancel() is
 * called, AFuture waits for the task, however, task's thread is still requested for interrupt. It's guarantees that
 * your task cannot be executed or be executing when AFuture destroyed and allows to efficiently utilize c++'s RAII
 * feature.
 *
 * To manage multiple AFutures, use AAsyncHolder and AFutureSet classes.
 *
 * When waiting for result, AFuture may execute the task on the caller thread instead of waiting. See AFuture::wait for
 * details.
 */
template<typename T = void>
class AFuture final: public aui::impl::future::Future<T> {
private:
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}
    ~AFuture() = default;

    void supplyResult(T v) const noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

        std::unique_lock lock(inner->mutex);
        inner->value = std::move(v);
        inner->cv.notify_all();
        AUI_NULLSAFE(inner->onSuccess)(*inner->value);
    }

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException() const noexcept {
        auto& inner = (*super::mInner);
        std::unique_lock lock(inner->mutex);
        inner->reportException();
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(const AFuture& r) const noexcept {
        return super::mInner == r.mInner;
    }

    template<typename Callback>
    const AFuture& onSuccess(Callback&& callback) const noexcept {
        super::onSuccess(std::forward<Callback>(callback));
        return *this;
    }

    template<typename Callback>
    const AFuture& onError(Callback&& callback) const noexcept {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }
};

template<>
class AFuture<void> final: public aui::impl::future::Future<void> {
private:
    using T = void;
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}
    ~AFuture() = default;

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException() const noexcept {
        auto& inner = (*super::mInner);
        std::unique_lock lock(inner->mutex);
        inner->reportException();
    }

    void supplyResult() const noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

        std::unique_lock lock(inner->mutex);
        inner->value = true;
        inner->cv.notify_all();
        AUI_NULLSAFE(inner->onSuccess)();
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(const AFuture& r) const noexcept {
        return super::mInner == r.mInner;
    }

    /**
     * @brief Add onSuccess callback to the future.
     * @details
     * The callback will be called on the worker's thread when the async task is returned a result.
     *
     * onSuccess does not expand AFuture's lifespan, so when AFuture becomes invalid, onSuccess would not be called.
     */
    template<typename Callback>
    const AFuture& onSuccess(Callback&& callback) const noexcept {
        super::onSuccess(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Add onSuccess callback to the future.
     * @details
     * The callback will be called on the worker's thread when the async task is returned a result.
     *
     * onSuccess does not expand AFuture's lifespan, so when AFuture becomes invalid, onSuccess would not be called.
     */
    template<typename Callback>
    const AFuture& onError(Callback&& callback) const noexcept {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }
};


#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
