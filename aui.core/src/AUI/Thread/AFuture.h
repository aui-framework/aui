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

#include <utility>
#include "AUI/Util/ABitField.h"
#if AUI_COROUTINES
#include <coroutine>
#endif

#include <atomic>
#include <functional>
#include <optional>
#include "AConditionVariable.h"
#include "AMutex.h"
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>
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
    JUST_WAIT = 0b00,
    ALLOW_STACKFULL_COROUTINES = 0b10,
    ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP = 0b01,
    DEFAULT = ALLOW_STACKFULL_COROUTINES | ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP,
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
                        assert(bool(func));
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

            void reportException() noexcept {
                if (cancelled) {
                    return;
                }
                std::unique_lock lock(mutex);
                exception.emplace();
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
                assert(lock.owns_lock());
                if (cancelled) {
                    return;
                }
                if (value && onSuccess) {
                    auto localOnSuccess = std::move(onSuccess);
                    onSuccess = nullptr;
                    lock.unlock();
                    try {
                        if constexpr (isVoid) {
                            localOnSuccess();
                        } else {
                            localOnSuccess(*value);
                        }
                    } catch (const AException& e) {
                        ALogger::err("AFuture") << "AFuture onSuccess thrown an exception: " << e;
                    }
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

        void reportException() const noexcept {
            (*mInner)->reportException();
        }

        template<typename Callback>
        void onSuccess(Callback&& callback) const noexcept {
            std::unique_lock lock((*mInner)->mutex);
            (*mInner)->addOnSuccessCallback(std::forward<Callback>(callback));
            (*mInner)->notifyOnSuccessCallback(lock);
        }

        template<aui::invocable<const AException&> Callback>
        void onError(Callback&& callback) const noexcept {
            std::unique_lock lock((*mInner)->mutex);
            (*mInner)->addOnErrorCallback(std::forward<Callback>(callback));
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
         *       yet. This behaviour can be disabled by <code>AFutureWait::JUST_WAIT</code> flag.
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
        typename FutureReturnType<Value>::type get(AFutureWait flags = AFutureWait::DEFAULT) {
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
         * @brief Returns the supplyResult from the another thread. Sleeps if the supplyResult is not currently available.
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
    };

}


/**
 * @brief Represents a value that is not currently available.
 * @ingroup core
 * @tparam T result type (void is default)
 * @details
 * AFuture is used as a result for asynchronous functions.
 *
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
 * If your operation consists of complex future sequences, you have multiple options:
 * 1. Use stackful coroutines. That is, you can use `operator*` and `get()` methods (blocking value acquiring) within a
 *    threadpool thread (including the one that runs @ref async 's body). If value is not currently available, these
 *    methods temporarily return the thread to threadpool, effeciently allowing it to execute other tasks.
 *    @note Be aware fot `std::unique_lock` and similar RAII-based lock functions when performing blocking value
 *          acquiring operation.
 * 2. Use stackless coroutines. C++20 introduced coroutines language feature. That is, you can use co_await operator to
 *    AFuture value:
 * @code{cpp}
 * AFuture<int> longOperation();
 * AFuture<int> myFunction() {
 *   int resultOfLongOperation = co_await longOperation();
 *   return resultOfLongOperation + 1;
 * }
 * @endcode
 * 3. Use AComplexFutureOperation. This class creates AFuture (root AFuture) and forwards all exceptions to the root
 *    AFuture. This method is not recommended for trivial usecases, as it requires you to extensivly youse onSuccess
 *    method in order to get and process AFuture result, leading your code to hardly maintainable spaghetti.
 *
 * @code{cpp}
 *
 * For rare cases, you can default-construct AFuture and the result can be supplied manually with the supplyResult() method:
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
 * @note Be aware of exceptions or control flow keywords! If you don't pass the result, AFuture will always stay
 *       unavailable, thus all waiting code will wait indefinitely long, leading to resource leaks (CPU and memory).
 *       Consider using one of suggested methods of usage instead.
 *
 * AFuture provides a set of functions for both "value emitting" side: supplyValue(), supplyException(), and "value
 * receiving" side:
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
 * When waiting for result, AFuture may execute the task (if not default-constructed) on the caller thread instead of
 * waiting. See AFuture::wait for details.
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

    AFuture(Task task = nullptr) noexcept: super(std::move(task)) {}
    ~AFuture() = default;

    /**
     * @brief Pushes the result to AFuture.
     * @param v value
     * @details
     * After AFuture grabbed the value, supplyResult calls onSuccess listeners with the new value.
     */
    void supplyResult(T v) const noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

        std::unique_lock lock(inner->mutex);
        inner->value = std::move(v);
        inner->cv.notify_all();
        inner->notifyOnSuccessCallback(lock);
    }

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException() const noexcept {
        auto& inner = (*super::mInner);
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

    /**
     * @brief Add onSuccess callback to the future.
     * @details
     * The callback will be called on the worker's thread when the async task is returned a result.
     *
     * onSuccess does not expand AFuture's lifespan, so when AFuture becomes invalid, onSuccess would not be called.
     *
     * @note
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * @code{cpp}
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * @endcode
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
     * @note
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * @code{cpp}
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * @endcode
     */
    template<aui::invocable<const AException&> Callback>
    const AFuture& onError(Callback&& callback) const noexcept {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @brief Maps this AFuture to another type of AFuture.
     */
    template<aui::invocable<const T&> Callback>
    auto map(Callback&& callback) -> AFuture<decltype(callback(std::declval<T>()))> const {
        AFuture<decltype(callback(std::declval<T>()))> result;
        onSuccess([result, callback = std::forward<Callback>(callback)](const T& v) {
            result.supplyResult(callback(v));
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

    /**
     * @brief Stores an exception from std::current_exception to the future.
     */
    void supplyException() const noexcept {
        auto& inner = (*super::mInner);
        inner->reportException();
    }

    /**
     * @brief Pushes "success" result.
     * @details
     * supplyResult calls onSuccess listeners with the new value.
     */
    void supplyResult() const noexcept {
        auto& inner = (*super::mInner);
        assert(("task is already provided", inner->task == nullptr));

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
     * @note
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * @code{cpp}
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * @endcode
     */
    template<aui::invocable Callback>
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
     *
     * @note
     * To expand lifespan, create an AAsyncHolder inside your window or object; then put the instance of AFuture there.
     * Example:
     * @code{cpp}
     * ...
     * private:
     *   AAsyncHolder mAsync;
     * ...
     *
     * mAsync << functionReturningFuture().onSuccess(...); // or onError
     * @endcode
     */
    template<aui::invocable<const AException&> Callback>
    const AFuture& onError(Callback&& callback) const noexcept {
        super::onError(std::forward<Callback>(callback));
        return *this;
    }
};


#include <AUI/Thread/AThreadPool.h>
#include <AUI/Common/AException.h>

template <typename Value>
void aui::impl::future::Future<Value>::Inner::wait(const _weak<CancellationWrapper<Inner>>& innerWeak,
                                                   ABitField<AFutureWait> flags) noexcept {
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

        if (flags & AFutureWait::ALLOW_STACKFULL_COROUTINES) {
            if (auto threadPoolWorker = _cast<AThreadPool::Worker>(AThread::current())) {
                auto callback = [threadPoolWorker](auto&&...) {
                    threadPoolWorker->threadPool().wakeUpAll();
                };
                addOnSuccessCallback(callback);
                addOnErrorCallback(callback);
                lock.unlock();
                threadPoolWorker->loop([&] { return !hasValue(); });

                return;
            }
        }
        while ((thread || !cancelled) && !hasResult()) {
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
        future.supplyResult(std::move(v));
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
