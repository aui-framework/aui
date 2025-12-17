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

#include <atomic>
#include <functional>
#include <optional>
#include <exception>
#include <thread>
#include <utility>
#if AUI_COROUTINES
#include <coroutine>
#endif

#include <AUI/Traits/concepts.h>
#include <AUI/Util/ABitField.h>
#include <AUI/Thread/AConditionVariable.h>
#include <AUI/Thread/AMutex.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>

class API_AUI_CORE AThreadPool;


class AInvocationTargetException: public AException {

public:
    AInvocationTargetException(const AString& message = {}, std::exception_ptr causedBy = std::current_exception()):
        AException(message, std::move(causedBy), AStacktrace::capture(3)) {}
    AString getMessage() const noexcept override {
        try {
            std::rethrow_exception(causedBy());
        } catch (const AException& e) {
            return AException::getMessage() + ": " + e.getMessage();
        } catch (const std::exception& e) {
            return AException::getMessage() + ": " + e.what();
        } catch (...) {
            return AException::getMessage();
        }
    }

    ~AInvocationTargetException() noexcept override = default;
};


/**
 * Controls <code>AFuture::wait</code> behaviour.
 * @see AFuture::wait
 */
AUI_ENUM_FLAG(AFutureWait) {
    JUST_WAIT = 0b00,
    ALLOW_STACKFUL_COROUTINES = 0b10,

    /**
     * @brief Use work stealing.
     */
    ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP = 0b01,
    DEFAULT = ALLOW_STACKFUL_COROUTINES | ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP,
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

    template<typename T>
    struct FutureReturnType {
        using type = T&;
    };

    template<>
    struct FutureReturnType<void> {
        using type = void;
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
            /**
             * Spinlock mutex is used here because AFuture makes frequent calls to mutex and never locks for a long
             * time. Durations are small enough so it is worth to just busy wait instead of making syscalls.
             */
            ASpinlockMutex mutex;
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

            void wait(const _weak<CancellationWrapper<Inner>>& innerWeak, ABitField<AFutureWait> flags = AFutureWait::DEFAULT) noexcept;

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
                        AUI_ASSERT(bool(func));
                        lock.unlock();
                        innerCancellation = nullptr;
                        if constexpr(isVoid) {
                            func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                lock.lock();
                                value = true;
                                cv.notify_all();
                                notifyOnSuccessCallback(lock);

                                (void)sharedPtrLock; // sharedPtrLock is *used*
                                if (lock.owns_lock()) lock.unlock(); // unlock earlier because destruction of shared_ptr may cause deadlock
                            }
                        } else {
                            auto result = func();
                            if (auto sharedPtrLock = innerWeak.lock()) {
                                lock.lock();
                                value = std::move(result);
                                cv.notify_all();
                                notifyOnSuccessCallback(lock);

                                (void)sharedPtrLock; // sharedPtrLock is *used*
                                if (lock.owns_lock()) lock.unlock(); // unlock earlier because destruction of shared_ptr may cause deadlock
                            }
                        }
                    } catch (const AThread::Interrupted&) {
                        if (auto sharedPtrLock = innerWeak.lock()) {
                            inner->reportInterrupted();
                        }
                        throw;
                    } catch (...) {
                        if (auto sharedPtrLock = innerWeak.lock()) {
                            inner->reportException();
                        }
                        return false;
                    } 
                }
                return true;
            }

            void reportInterrupted() noexcept {
                std::unique_lock lock(mutex);
                interrupted = true;
                cv.notify_all();
            }

            void reportException(std::exception_ptr causedBy = std::current_exception()) noexcept {
                if (cancelled) {
                    return;
                }
                std::unique_lock lock(mutex);
                exception.emplace("exception reported", std::move(causedBy));
                cv.notify_all();
                if (!onError) {
                    return;
                }
                auto localOnError = std::move(onError);
                lock.unlock();
                localOnError(*exception);
            }


            /**
             * @brief Calls onSuccess callback.
             * @param lock lock of Inner::mutex mutex.
             * @details
             * lock is expected to be locked. Under the lock, callback is moved to local stack, lock is unlocked, and
             * only then callback is called. This helps to avoid deadlocks (i.e. retreiving AFuture's value in
             * onSuccess callback). lock is not locked again. If AFuture does not have value or onSuccess callback,
             * the lock remains untouched.
             */
            void notifyOnSuccessCallback(std::unique_lock<decltype(mutex)>& lock) noexcept {
                AUI_ASSERT(lock.owns_lock());
                if (cancelled) {
                    return;
                }
                if (value && onSuccess) {
                    auto localOnSuccess = std::move(onSuccess);
                    onSuccess = nullptr;
                    lock.unlock();
                    invokeOnSuccessCallback(localOnSuccess);
                }
            }

            template<typename F>
            void invokeOnSuccessCallback(F&& f) {
                try {
                    if constexpr (isVoid) {
                        f();
                    } else {
                        f(*value);
                    }
                } catch (const AException& e) {
                    ALogger::err("AFuture") << "AFuture onSuccess thrown an exception: " << e;
                }
            }

            template<typename Callback>
            void addOnSuccessCallback(Callback&& callback) {
                if constexpr (isVoid) {
                    if (onSuccess) {
                        onSuccess = [prev = std::move(onSuccess),
                                     callback = std::forward<Callback>(callback)]() mutable {
                            prev();
                            callback();
                        };
                    } else {
                        onSuccess = [callback = std::forward<Callback>(callback)]() mutable { callback(); };
                    }
                } else {
                    if (onSuccess) {
                        onSuccess = [prev = std::move(onSuccess),
                                     callback = std::forward<Callback>(callback)](const Value& v) mutable {
                            prev(v);
                            callback(v);
                        };
                    } else {
                        onSuccess = [callback = std::forward<Callback>(callback)](const Value& v) mutable {
                            callback(v);
                        };
                    }
                }
            }

            template<typename Callback>
            void addOnErrorCallback(Callback&& callback) {
                if (onError) {
                    onError = [prev = std::move(onError),
                               callback = std::forward<Callback>(callback)](const AException& v) {
                        prev(v);
                        callback(v);
                    };
                } else {
                    onError = [callback = std::forward<Callback>(callback)](const AException& v) { callback(v); };
                }
            }
        };

#if AUI_COROUTINES
        /**
         * @brief promise_type for C++ coroutines TS.
         */
        struct CoPromiseType;
#endif

    protected:
        _<CancellationWrapper<Inner>> mInner;


    public:
        /**
         * @param task a callback which will be executed by Future::Inner::tryExecute. Can be null. If null, the result
         *        should be provided by AFuture::supplyValue function.
         */
        Future(TaskCallback task = nullptr): mInner(_new<CancellationWrapper<Inner>>((_unique<Inner>)(new Inner(std::move(task))))) {}

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
         * @return true if the value or exception or interruption was received.
         */
        [[nodiscard]]
        bool hasResult() const noexcept {
            return (*mInner)->hasResult();
        }

        /**
         * @return true if asynchronous operation was successfuly completed and supplied a value, which can be obtained
         * without waiting.
         */
        [[nodiscard]]
        bool hasValue() const noexcept {
            return (*mInner)->hasValue();
        }

        void reportException() const noexcept {
            (*mInner)->reportException();
        }

        template<typename Callback>
        void onSuccess(Callback&& callback) const {
            if (hasValue()) { // cheap lookahead
                (*mInner)->invokeOnSuccessCallback(std::forward<Callback>(callback));
                return;
            }
            std::unique_lock lock((*mInner)->mutex);
            if (hasValue()) { // not so cheap, but cheapier than adding the callback to inner
                (*mInner)->invokeOnSuccessCallback(std::forward<Callback>(callback));
                return;
            }
            (*mInner)->addOnSuccessCallback(std::forward<Callback>(callback));
        }

        template<aui::invocable<const AException&> Callback>
        void onError(Callback&& callback) const {
            std::unique_lock lock((*mInner)->mutex);
            (*mInner)->addOnErrorCallback(std::forward<Callback>(callback));
        }

        /**
         * @brief Adds the callback to both onSuccess and onResult.
         */
        template<aui::invocable Callback>
        void onFinally(Callback&& callback) const {
            std::unique_lock lock((*mInner)->mutex);
            (*mInner)->addOnSuccessCallback([callback](const auto&...) { callback(); });
            (*mInner)->addOnErrorCallback([callback = std::move(callback)](const auto&...) { callback(); });
            (*mInner)->notifyOnSuccessCallback(lock);
        }

        /**
         * @brief Cancels the AFuture's task.
         * @details
         * When cancel() is called, there are 3 possible cases:
         * 1. AFuture's task is not taken by AThreadPool. The task is simply removed from the AThreadPool queue.
         * 2. AFuture's task is being executed. The task's thread is
         * [requested for interrupt](AAbstractThread::interrupt()).
         * 3. AFuture's task is already completed. cancel() does nothing.
         */
        void cancel() const noexcept {
            (*mInner)->cancel();
        }

        void reportInterrupted() const {
            (*mInner)->reportInterrupted();
        }

        /**
         * @brief Sleeps if the supplyValue is not currently available.
         * @details
         * The task will be executed inside wait() function if the threadpool have not taken the task to execute
         * yet. This behaviour can be disabled by <code>AFutureWait::JUST_WAIT</code> flag.
         */
        void wait(AFutureWait flags = AFutureWait::DEFAULT) const {
            (*mInner)->wait(mInner, flags);
            checkForSelfWait();
        }

        /**
         * @brief Returns the supplyValue from the another thread. Sleeps if the supplyValue is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        typename FutureReturnType<Value>::type get(AFutureWait flags = AFutureWait::DEFAULT) const {
            AThread::interruptionPoint();

            (*mInner)->wait(mInner, flags);

            AThread::interruptionPoint();
            if ((*mInner)->exception) {
                throw *(*mInner)->exception;
            }
            if ((*mInner)->interrupted) {
                throw AInvocationTargetException("Future execution interrupted");
            }
            checkForSelfWait();
            if constexpr(!isVoid) {
                return *(*mInner)->value;
            }
        }


        /**
         * @brief Returns the task result from the another thread. Sleeps if the task result is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        typename FutureReturnType<Value>::type operator*() const {
            return **const_cast<Future*>(this);
        }

        /**
         * @brief Returns the supplyValue from the another thread. Sleeps if the supplyValue is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        typename FutureReturnType<Value>::type operator*() {
            return get();
        }

        /**
         * @brief Returns the supplyValue from the another thread. Sleeps if the supplyValue is not currently available.
         * <dl>
         *   <dt><b>Sneaky exceptions</b></dt>
         *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception.</dd>
         * </dl>
         * @return the object stored from the another thread.
         */
        Value* operator->() const {
            return &operator*();
        }

    private:
        void checkForSelfWait() const {
            if (!(*mInner)->hasResult() && AThread::current() == (*mInner)->thread) {
                throw AException("self wait?");
            }
        }
    };

}


/**
 * @brief Represents a value that will be available at some point in the future.
 * @ingroup core
 * @tparam T result type (void is default)
 * @details
 * <!-- aui:experimental -->
 * AFuture is used as a result for asynchronous functions.
 *
 * AFuture is returned by [AUI_THREADPOOL] keyword, which is used to perform heavy operations in a background thread.
 *
 * ```cpp
 * AFuture<int> theFuture = AUI_THREADPOOL {
 *   AThread::sleep(1000); // long operation
 *   return 123;
 * };
 * ...
 * cout << *theFuture; // waits for the task, outputs 123
 * ```
 *
 * If your operation consists of complex future sequences, you have multiple options:
 *
 * 1.  Use stackful coroutines. That is, you can use `operator*` and `get()` methods (blocking value acquiring) within a
 *     threadpool thread (including the one that runs [AUI_THREADPOOL] 's body). If value is not currently available,
 *     these methods temporarily return the thread to threadpool, effeciently allowing it to execute other tasks.
 *
 *     Be aware fot `std::unique_lock` and similar RAII-based lock functions when performing blocking value
 *     acquiring operation.
 *
 * 2.  Use stackless coroutines. C++20 introduced coroutines language feature. That is, you can use co_await operator to
 *     AFuture value:
 *     ```cpp
 *     AFuture<int> longOperation();
 *     AFuture<int> myFunction() {
 *       int resultOfLongOperation = co_await longOperation();
 *       return resultOfLongOperation + 1;
 *     }
 *     ```
 *
 * 3.  Use AComplexFutureOperation. This class creates AFuture (root AFuture) and forwards all exceptions to the root
 *     AFuture. This method is not recommended for trivial usecases, as it requires you to extensivly youse onSuccess
 *     method in order to get and process AFuture result, leading your code to hardly maintainable spaghetti.
 *
 * For rare cases, you can default-construct AFuture and the result can be supplied manually with the supplyValue() method:
 *
 * ```cpp
 * AFuture<int> theFuture;
 * AThread t([=] {
 *   AThread::sleep(1000); // long operation
 *   theFuture.supplyValue(123);
 * });
 * t.start();
 * cout << *theFuture; // 123
 * ```
 *
 * Be aware of exceptions or control flow keywords! If you don't pass the result, AFuture will always stay
 * unavailable, thus all waiting code will wait indefinitely long, leading to resource leaks (CPU and memory).
 * Consider using one of suggested methods of usage instead.
 *
 * AFuture provides a set of functions for both "value emitting" side: supplyValue(), supplyException(), and "value
 * receiving" side: operator->(), operator*(), get().
 *
 * When AFuture's operation is completed it calls either onSuccess() or onError(). These callbacks are excepted to be
 * called in any case. Use onFinally() to handle both.
 *
 * AFuture is a shared_ptr-based wrapper so it can be easily copied, pointing to the same task.
 *
 * If all AFutures of the task are destroyed, the task is cancelled. If the task is executing when cancel() is
 * called, AFuture waits for the task, however, task's thread is still requested for interrupt. It guarantees that
 * your task cannot be executed or be executing when AFuture destroyed and allows to efficiently utilize c++'s RAII
 * feature.
 *
 * To manage multiple AFutures, use AAsyncHolder or AFutureSet classes.
 *
 * AFuture implements work-stealing algorithm to prevent deadlocks and optimizaze thread usage: when waiting for result,
 * AFuture may execute the task (if not default-constructed) on the caller thread instead of waiting. See AFuture::wait
 * for details.
 */
template<typename T = void>
class AFuture final: public aui::impl::future::Future<T> {
private:
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;
#if AUI_COROUTINES
    using promise_type = typename super::CoPromiseType;
#endif
    using Inner = aui::impl::future::CancellationWrapper<typename super::Inner>;

    explicit AFuture(T immediateValue): super() {
        auto& inner = (*super::mInner);
        inner->value = std::move(immediateValue);
    }
    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}
    ~AFuture() = default;

    AFuture(const AFuture&) = default;
    AFuture(AFuture&&) noexcept = default;

    AFuture& operator=(const AFuture&) = default;
    AFuture& operator=(AFuture&&) noexcept = default;

    /**
     * @brief Pushes the result to AFuture.
     * @param v value
     * @details
     * After AFuture grabbed the value, supplyValue calls onSuccess listeners with the new value.
     */
    void supplyValue(T v) const noexcept {
        auto& inner = (*super::mInner);
        AUI_ASSERTX(inner->task == nullptr, "task is already provided");

        std::unique_lock lock(inner->mutex);
        inner->value = std::move(v);
        inner->cv.notify_all();
        inner->notifyOnSuccessCallback(lock);
    }

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException(std::exception_ptr causedBy = std::current_exception()) const noexcept {
        auto& inner = (*super::mInner);
        inner->reportException(std::move(causedBy));
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(std::nullptr_t) const noexcept {
        return super::mInner == nullptr;
    }

    [[nodiscard]]
    bool operator!=(std::nullptr_t) const noexcept {
        return super::mInner != nullptr;
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
     *
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * ```cpp
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * ```
     */
    template<aui::invocable<const T&> Callback>
    const AFuture& onSuccess(Callback&& callback) const noexcept {
        super::onSuccess(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Add onError callback to the future.
     * @details
     * The callback will be called on the worker's thread when the async task is returned a result.
     *
     * onError does not expand AFuture's lifespan, so when AFuture becomes invalid, onSuccess would not be called.
     *
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * ```cpp
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * ```
     */
    template<aui::invocable<const AException&> Callback>
    const AFuture& onError(Callback&& callback) const noexcept {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Adds the callback to both onSuccess and onResult.
     */
    template<aui::invocable Callback>
    const AFuture& onFinally(Callback&& callback) const noexcept {
        super::onFinally(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Maps this AFuture to another type of AFuture.
     */
    template<aui::invocable<const T&> Callback>
    auto map(Callback&& callback) -> AFuture<decltype(callback(std::declval<T>()))> const {
        AFuture<decltype(callback(std::declval<T>()))> result;
        onSuccess([result, callback = std::forward<Callback>(callback)](const T& v) {
            result.supplyValue(callback(v));
        });
        onError([result](const AException& v) {
            try {
                throw v;
            } catch (...) {
                result.reportException();
            }
        });
        return result;
    }
};

template<>
class AFuture<void> final: public aui::impl::future::Future<void> {
private:
    using T = void;
    using super = typename aui::impl::future::Future<T>;

public:
    using Task = typename super::TaskCallback;
#if AUI_COROUTINES
    using promise_type = typename super::CoPromiseType;
#endif
    using Inner = decltype(std::declval<super>().inner());

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}
    ~AFuture() = default;

    AFuture(const AFuture&) = default;
    AFuture(AFuture&&) noexcept = default;

    AFuture& operator=(const AFuture&) = default;
    AFuture& operator=(AFuture&&) noexcept = default;

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException(std::exception_ptr causedBy = std::current_exception()) const noexcept {
        auto& inner = (*super::mInner);
        inner->reportException(std::move(causedBy));
    }

    /**
     * @brief Pushes "success" result.
     * @details
     * supplyValue calls onSuccess listeners with the new value.
     */
    void supplyValue() const noexcept {
        auto& inner = (*super::mInner);
        AUI_ASSERTX(inner->task == nullptr, "task is already provided");

        std::unique_lock lock(inner->mutex);
        inner->value = true;
        inner->cv.notify_all();
        inner->notifyOnSuccessCallback(lock);
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        super::mInner = nullptr;
        return *this;
    }

    [[nodiscard]]
    bool operator==(std::nullptr_t) const noexcept {
        return super::mInner == nullptr;
    }

    [[nodiscard]]
    bool operator!=(std::nullptr_t) const noexcept {
        return super::mInner != nullptr;
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
     *
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * ```cpp
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * ```
     */
    template<aui::invocable Callback>
    const AFuture& onSuccess(Callback&& callback) const {
        super::onSuccess(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Add onSuccess callback to the future.
     * @details
     * The callback will be called on the worker's thread when the async task is returned a result.
     *
     * onSuccess does not expand AFuture's lifespan, so when AFuture becomes invalid, onSuccess would not be called.
     *
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * ```cpp
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * ```
     */
    template<aui::invocable<const AException&> Callback>
    const AFuture& onError(Callback&& callback) const {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Adds the callback to both onSuccess and onResult.
     */
    template<aui::invocable Callback>
    const AFuture& onFinally(Callback&& callback) const {
        super::onFinally(std::forward<Callback>(callback));
        return *this;
    }
};

template <typename Value>
void aui::impl::future::Future<Value>::Inner::wait(const _weak<CancellationWrapper<Inner>>& innerWeak,
                                                   ABitField<AFutureWait> flags) noexcept {
    if (hasResult()) return; // cheap check
    std::unique_lock lock(mutex);
    try {
        if ((thread || !cancelled) && !hasResult() && flags & AFutureWait::ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP && task) {
            // task is not have picked up by the threadpool; execute it here
            lock.unlock();
            if (tryExecute(innerWeak)) {
                return;
            }
            lock.lock();
        }

        if (flags & AFutureWait::ALLOW_STACKFUL_COROUTINES) {
            if (auto threadPoolWorker = _cast<AThreadPool::Worker>(AThread::current())) {
                if (hasResult()) return; // for sure
                AUI_ASSERT(lock.owns_lock());
                auto callback = [threadPoolWorker](auto&&...) {
                    threadPoolWorker->threadPool().wakeUpAll();
                };
                addOnSuccessCallback(callback);
                addOnErrorCallback(callback);

                threadPoolWorker->loop([&] {
                  if (lock.owns_lock())
                    lock.unlock();
                  return !hasResult();
                });

                return;
            }
        }
        while ((thread || !cancelled) && !hasResult()) {
            if (thread == AThread::current()) [[unlikely]] {
                // self wait?
                return;
            }
            cv.wait(lock);
        }
    } catch (const AThread::Interrupted& e) {
        e.needRethrow();
    }
}

#if AUI_COROUTINES
template<typename Value>
struct aui::impl::future::Future<Value>::CoPromiseType {
    AFuture<Value> future;
    auto initial_suspend() const noexcept
    {
        return std::suspend_never{};
    }

    auto final_suspend() const noexcept
    {
        return std::suspend_never{};
    }
    auto unhandled_exception() const noexcept {
        future.supplyException();
    }

    const AFuture<Value>& get_return_object() const noexcept {
        return future; 
    }

    void return_value(Value v) const noexcept {
        future.supplyValue(std::move(v));
    }
};

template<typename T>
auto operator co_await(AFuture<T> future) {
    struct Awaitable {
        AFuture<T> future;

        bool await_ready() const noexcept {
            return future.hasResult();
        }

        T await_resume() {
            return *future;
        }


        void await_suspend(std::coroutine_handle<> h)
        {
            future.onSuccess([h](const int&) {
                h.resume();
            });
            
            future.onError([h](const AException&) {
                h.resume();
            });
        }
    };

    return Awaitable{ std::move(future) };
}
#endif
