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
#include <chrono>
#include <tuple>
#include <variant>
#if AUI_COROUTINES
#include <coroutine>
#endif

#include <AUI/Common/AException.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Thread/AConditionVariable.h>
#include <AUI/Thread/AMutex.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Thread/AFutureWait.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Util/ABitField.h>

class AThreadPool;

/**
 * @brief An exception thrown by an asynchronous operation, as seen by those who wait for it.
 * @ingroup core
 * @details
 * The original exception is the cause (`causedBy()`). Each future between the throw and the waiter adds one level, so
 * the chain of causes shows the whole path through nested coroutines.
 */
class AInvocationTargetException: public AException {

public:
    AInvocationTargetException(const AString& message = {}, std::exception_ptr causedBy = std::current_exception()):
        AException(message, std::move(causedBy), AStacktrace::capture(3)) {}
    AString getMessage() const noexcept override {
        if (causedBy() == nullptr) {
            return AException::getMessage();
        }
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
 * @brief The exception an [AFuture] fails with when every [AFuture::Resolver] of it is destroyed without a result.
 * @ingroup core
 * @details
 * A producer that forgets to call `supplyValue` or `supplyException`, or loses the resolver on an early return or an
 * exception, would otherwise leave the waiting side hanging forever.
 */
class AFutureAbandonedException : public AException {
public:
    AFutureAbandonedException() : AException("the future was abandoned: its resolver was destroyed without a result") {}
};

/**
 * @brief The cause of the exception [AFuture::withTimeout] fails with when the future does not complete in time.
 * @ingroup core
 */
class ATimeoutException : public AException {
public:
    ATimeoutException(const AString& message = "timed out") : AException(message, nullptr) {}
    ~ATimeoutException() noexcept override = default;
};

namespace aui::impl::future {

/**
 * @brief Type a future of `T` stores; `std::monostate` for `void`, so `void` futures fit into tuples and variants.
 */
template <typename T>
using Storage = std::conditional_t<std::is_void_v<T>, std::monostate, T>;

template <typename T>
struct ReturnType {
    using type = T&;
};

template <>
struct ReturnType<void> {
    using type = void;
};

template <typename F, typename T>
concept OnValueCallback =
    (std::is_void_v<T> && aui::invocable<F>) || (!std::is_void_v<T> && aui::invocable<F, const Storage<T>&>);

/**
 * @brief Placeholder for a callback `Inner::subscribe` should not register.
 */
struct NoCallback {};

// The exception to pass on when a dependent future fails because of `e`. Keeps the exact AInvocationTargetException,
// so the chain of causes leads through every future of a wait chain.
inline std::exception_ptr causeOf(const AException& e) {
    if (auto ite = dynamic_cast<const AInvocationTargetException*>(&e)) {
        return std::make_exception_ptr(*ite);
    }
    return std::make_exception_ptr(e);
}

inline std::exception_ptr cancelledException() { return std::make_exception_ptr(AException("the future is cancelled")); }

template <typename F, typename... Args>
void invokeCallback(F& callback, const char* name, Args&&... args) noexcept {
    try {
        std::invoke(callback, std::forward<Args>(args)...);
    } catch (const AException& e) {
        ALogger::err("AFuture") << name << " callback has thrown an exception: " << e;
    } catch (const std::exception& e) {
        ALogger::err("AFuture") << name << " callback has thrown an exception: " << e.what();
    } catch (...) {
        ALogger::err("AFuture") << name << " callback has thrown an exception";
    }
}

/**
 * @brief State shared by AFuture and its AFuture::Resolver.
 * @details
 * Three counters share one word: futures (the consumer side), resolvers (the producer side, including a task that has
 * not started yet) and internal references held for the duration of an operation. The state is deleted when the word
 * becomes zero. The last future cancels the operation; the last resolver of a pending state abandons it.
 */
template <typename T>
struct Inner {
    enum class State : std::uint8_t {
        PENDING,
        VALUE,
        EXCEPTION,
        CANCELLED,
    };
    static constexpr bool isVoid = std::is_void_v<T>;
    using Task = std::function<T()>;
    using OnValue = std::conditional_t<isVoid, std::function<void()>, std::function<void(const Storage<T>&)>>;
    using OnException = std::function<void(const AException&)>;

    static constexpr std::uint64_t FUTURE = 1;
    static constexpr std::uint64_t BITS = 21;
    static constexpr std::uint64_t RESOLVER = std::uint64_t(1) << BITS;
    static constexpr std::uint64_t INTERNAL = RESOLVER << BITS;
    static constexpr std::uint64_t MASK = RESOLVER - 1;

    std::atomic<std::uint64_t> counters;
    std::atomic<State> state = State::PENDING;
    std::atomic_bool executing = false;

    /**
     * Spinlock mutex is used here because AFuture makes frequent calls to mutex and never locks for a long
     * time. Durations are small enough so it is worth to just busy wait instead of making syscalls.
     */
    ASpinlockMutex mutex;
    // created by the first blocking wait(): most futures are never waited for this way, and a condition variable costs
    // an allocation, a syscall-backed destructor and a lock on every notify
    std::unique_ptr<AConditionVariable> cv;
    Task task;
    _<AAbstractThread> thread;
    bool keptAlive = false;
    std::conditional_t<isVoid, std::monostate, AOptional<T>> value;
    AOptional<AInvocationTargetException> exception;
    OnValue onValue;
    OnException onException;
    std::function<void()> onCancel;

    explicit Inner(std::uint64_t counters, Task task = nullptr) noexcept
      : counters(counters), task(std::move(task)) {}

    // under mutex
    void notifyWaiters() noexcept {
        if (cv) {
            cv->notify_all();
        }
    }

    void acquireFuture() noexcept { counters.fetch_add(FUTURE, std::memory_order_relaxed); }
    void acquireResolver() noexcept { counters.fetch_add(RESOLVER, std::memory_order_relaxed); }

    void releaseFuture() noexcept {
        auto old = counters.load(std::memory_order_relaxed);
        for (;;) {
            if ((old & MASK) == FUTURE) {
                // the last future: hold the state while cancelling
                if (counters.compare_exchange_weak(
                        old, old - FUTURE + INTERNAL, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                    cancel();
                    releaseInternal();
                    return;
                }
            } else if (counters.compare_exchange_weak(
                           old, old - FUTURE, std::memory_order_release, std::memory_order_relaxed)) {
                return;
            }
        }
    }

    void releaseResolver() noexcept {
        auto old = counters.load(std::memory_order_relaxed);
        for (;;) {
            if (((old >> BITS) & MASK) == 1) {
                // the last resolver: hold the state while abandoning
                if (counters.compare_exchange_weak(
                        old, old - RESOLVER + INTERNAL, std::memory_order_acq_rel, std::memory_order_relaxed)) {
                    abandon();
                    releaseInternal();
                    return;
                }
            } else if (counters.compare_exchange_weak(
                           old, old - RESOLVER, std::memory_order_release, std::memory_order_relaxed)) {
                return;
            }
        }
    }

    void releaseInternal() noexcept {
        if (counters.fetch_sub(INTERNAL, std::memory_order_acq_rel) == INTERNAL) {
            delete this;
        }
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return state.load(std::memory_order_acquire) == State::VALUE;
    }

    [[nodiscard]]
    bool hasException() const noexcept {
        return state.load(std::memory_order_acquire) == State::EXCEPTION;
    }

    [[nodiscard]]
    bool hasResult() const noexcept {
        auto s = state.load(std::memory_order_acquire);
        return s == State::VALUE || s == State::EXCEPTION;
    }

    [[nodiscard]]
    bool isCancelled() const noexcept {
        return state.load(std::memory_order_acquire) == State::CANCELLED;
    }

    /**
     * @return true if nothing is going to happen to the state anymore, so waiting is over.
     */
    [[nodiscard]]
    bool isSettled() const noexcept {
        switch (state.load(std::memory_order_acquire)) {
            case State::PENDING:
                return false;
            case State::CANCELLED:
                return !executing.load(std::memory_order_acquire);
            default:
                return true;
        }
    }

    template <typename... Args>
    void supplyValue(Args&&... args) noexcept {
        if (state.load(std::memory_order_acquire) != State::PENDING) {
            return;
        }
        std::unique_lock lock(mutex);
        if (state.load(std::memory_order_relaxed) != State::PENDING) {
            return;
        }
        if constexpr (!isVoid) {
            value.emplace(std::forward<Args>(args)...);
        }
        state.store(State::VALUE, std::memory_order_release);
        auto localOnValue = std::exchange(onValue, nullptr);
        auto localOnException = std::exchange(onException, nullptr);
        auto localOnCancel = std::exchange(onCancel, nullptr);
        bool releaseKeepAlive = std::exchange(keptAlive, false);
        notifyWaiters();
        lock.unlock();
        if (localOnValue) {
            if constexpr (isVoid) {
                invokeCallback(localOnValue, "onValue");
            } else {
                invokeCallback(localOnValue, "onValue", *value);
            }
        }
        if (releaseKeepAlive) {
            releaseFuture();
        }
    }

    void supplyException(std::exception_ptr causedBy) noexcept {
        if (state.load(std::memory_order_acquire) != State::PENDING) {
            return;
        }
        std::unique_lock lock(mutex);
        if (state.load(std::memory_order_relaxed) != State::PENDING) {
            return;
        }
        exception.emplace("exception reported", std::move(causedBy));
        state.store(State::EXCEPTION, std::memory_order_release);
        auto localOnValue = std::exchange(onValue, nullptr);
        auto localOnException = std::exchange(onException, nullptr);
        auto localOnCancel = std::exchange(onCancel, nullptr);
        bool releaseKeepAlive = std::exchange(keptAlive, false);
        notifyWaiters();
        lock.unlock();
        if (localOnException) {
            invokeCallback(localOnException, "onException", *exception);
        }
        if (releaseKeepAlive) {
            releaseFuture();
        }
    }

    void abandon() noexcept {
        if (state.load(std::memory_order_acquire) != State::PENDING) {
            return;
        }
        supplyException(std::make_exception_ptr(AFutureAbandonedException()));
    }

    void cancel() noexcept {
        if (state.load(std::memory_order_acquire) != State::PENDING) {
            return;
        }
        std::unique_lock lock(mutex);
        if (state.load(std::memory_order_relaxed) != State::PENDING) {
            return;
        }
        state.store(State::CANCELLED, std::memory_order_release);
        auto localTask = std::exchange(task, nullptr);
        auto localOnValue = std::exchange(onValue, nullptr);
        auto localOnException = std::exchange(onException, nullptr);
        auto localOnCancel = std::exchange(onCancel, nullptr);
        bool releaseKeepAlive = std::exchange(keptAlive, false);
        if (executing && thread) {
            thread->interrupt();
        }
        notifyWaiters();
        lock.unlock();
        bool hadTask = bool(localTask);
        localTask = nullptr;
        if (localOnCancel) {
            invokeCallback(localOnCancel, "onCancel");
        }
        if (hadTask) {
            releaseResolver();   // the task that is not going to run was a producer
        }
        if (releaseKeepAlive) {
            releaseFuture();
        }
    }

    void keepAlive() noexcept {
        std::unique_lock lock(mutex);
        if (state.load(std::memory_order_relaxed) != State::PENDING || keptAlive) {
            return;
        }
        keptAlive = true;
        acquireFuture();
    }

    /**
     * @brief Registers callbacks for the value, the exception and the cancellation; or calls the matching one now if
     * the state is settled already. Pass NoCallback to skip one.
     */
    template <typename V, typename E, typename C>
    void subscribe(V&& v, E&& e, C&& c) {
        std::unique_lock lock(mutex);
        switch (state.load(std::memory_order_relaxed)) {
            case State::PENDING:
                if constexpr (!std::is_same_v<std::decay_t<V>, NoCallback>) {
                    append(onValue, std::forward<V>(v));
                }
                if constexpr (!std::is_same_v<std::decay_t<E>, NoCallback>) {
                    append(onException, std::forward<E>(e));
                }
                if constexpr (!std::is_same_v<std::decay_t<C>, NoCallback>) {
                    append(onCancel, std::forward<C>(c));
                }
                return;
            case State::VALUE:
                lock.unlock();
                if constexpr (!std::is_same_v<std::decay_t<V>, NoCallback>) {
                    if constexpr (isVoid) {
                        invokeCallback(v, "onValue");
                    } else {
                        invokeCallback(v, "onValue", *value);
                    }
                }
                return;
            case State::EXCEPTION:
                lock.unlock();
                if constexpr (!std::is_same_v<std::decay_t<E>, NoCallback>) {
                    invokeCallback(e, "onException", *exception);
                }
                return;
            case State::CANCELLED:
                lock.unlock();
                if constexpr (!std::is_same_v<std::decay_t<C>, NoCallback>) {
                    invokeCallback(c, "onCancel");
                }
                return;
        }
    }

    template <typename Function, typename Callback>
    static void append(Function& slot, Callback&& callback) {
        if (!slot) {
            slot = std::forward<Callback>(callback);
            return;
        }
        slot = [prev = std::move(slot), callback = std::forward<Callback>(callback)](const auto&... args) mutable {
            prev(args...);
            callback(args...);
        };
    }

    /**
     * @brief Executes the task if nobody has taken it yet.
     * @return true if this call has executed the task.
     */
    bool tryExecute() {
        std::unique_lock lock(mutex);
        if (state.load(std::memory_order_relaxed) != State::PENDING || !task) {
            return false;
        }
        auto func = std::exchange(task, nullptr);
        executing = true;
        thread = AThread::current();
        lock.unlock();

        // this call owns the resolver reference of the task from now on
        struct Finish {
            Inner* self;
            ~Finish() {
                {
                    std::unique_lock lock(self->mutex);
                    self->executing = false;
                    self->thread = nullptr;
                    if (self->state.load(std::memory_order_relaxed) == State::CANCELLED) {
                        // isSettled() of a cancelled future waits for the task to stop
                        self->notifyWaiters();
                    }
                }
                self->releaseResolver();
            }
        } finish { this };

        try {
            if constexpr (isVoid) {
                func();
                supplyValue();
            } else {
                supplyValue(func());
            }
        } catch (const AThread::Interrupted&) {
            supplyException(std::current_exception());   // no-op if cancelled
            throw;
        } catch (...) {
            supplyException(std::current_exception());
        }
        return true;
    }

    void checkForSelfWait() {
        if (!hasResult() && executing && AThread::current() == thread) {
            throw AException("self wait?");
        }
    }

    void wait(ABitField<AFutureWait> flags);
};

#if AUI_COROUTINES
template <typename T>
struct CoPromiseType;
#endif

struct Access;

}   // namespace aui::impl::future

// a string literal is required by [[nodiscard]]
#define AUI_IMPL_FUTURE_DISCARDED                                                                                      \
    "the operation is cancelled when its last AFuture is destroyed; keep the result, put it into AAsyncHolder or "     \
    "call keepAlive()"

/**
 * @brief Represents a value that will be available at some point in the future.
 * @ingroup core
 * @tparam T result type (void is default)
 * @details
 * <!-- aui:experimental -->
 * AFuture is the result of an asynchronous operation: a task on a thread pool, a coroutine, a network request, a
 * dialog. It is the C++ counterpart of a JavaScript `Promise`, a Kotlin `Deferred` or a C# `Task`. See [core-async]
 * for an overview and a comparison with those.
 *
 * An AFuture is obtained from one of these:
 *
 * - [AUI_THREADPOOL] runs a lambda on the thread pool:
 *   ```cpp
 *   AFuture<int> theFuture = AUI_THREADPOOL {
 *     AThread::sleep(1000); // long operation
 *     return 123;
 *   };
 *   ```
 * - A coroutine, a function returning AFuture that uses `co_await` or `co_return`:
 *   ```cpp
 *   AFuture<int> longOperation();
 *   AFuture<int> myFunction() {
 *     int resultOfLongOperation = co_await longOperation();
 *     co_return resultOfLongOperation + 1;
 *   }
 *   ```
 * - [AFuture::create] wraps a callback-based API, like `new Promise((resolve, reject) => ...)` in JavaScript:
 *   ```cpp
 *   AFuture<APath> browseForFile() {
 *     return AFuture<APath>::create([](AFuture<APath>::Resolver resolver) {
 *       nativeDialog.show([resolver](APath path) { resolver.supplyValue(std::move(path)); });
 *     });
 *   }
 *   ```
 * - [AFuture::fromValue] and [AFuture::fromException] make a future that is complete already.
 *
 * The producer side ([AFuture::Resolver]) supplies the result; the consumer side (AFuture) gets it. The result is
 * set once: the first `supplyValue` or `supplyException` wins, the later ones are ignored. If every resolver is
 * destroyed before the result is supplied, the future fails with [AFutureAbandonedException] instead of waiting
 * forever.
 *
 * The consumer either blocks with `operator*`, `get()` or `wait()`, awaits with `co_await`, or subscribes with
 * `onValue()`, `onException()` and `onFinally()`. These callbacks are called exactly once: during resolution, on the
 * thread that supplies the result; or right away if the future is complete at subscription time.
 *
 * Exceptions are wrapped into AInvocationTargetException. Each future of a `co_await` chain adds one level, so the
 * chain of causes leads from the outermost coroutine to the original exception.
 *
 * AFuture is a reference counted handle, so it is cheap to copy. When the last AFuture of an operation is destroyed,
 * the operation is cancelled: a pending thread pool task is removed from the queue, a running one gets its thread
 * [interrupted](AAbstractThread::interrupt()), a coroutine stops at its next `co_await`, and callbacks are not called.
 * This is how a window that closes cancels the requests it has started. For "fire and forget", call
 * [AFuture::keepAlive]; to hold several futures, use AAsyncHolder.
 *
 * When blocking on a pool thread, `operator*`, `get()` and `wait()` work as stackful coroutines: the thread executes
 * other tasks of the pool until the result is available. Be aware of `std::unique_lock` and similar RAII-based locks
 * held during such a wait. If the awaited task has not been picked up by the pool yet, the waiting thread executes it
 * itself. See [AFuture::wait] for details.
 */

template <typename T>
class AFuture final {
private:
    using Inner = aui::impl::future::Inner<T>;
    using NonVoid = aui::impl::future::Storage<T>;
    static constexpr bool isVoid = std::is_void_v<T>;

    template <typename>
    friend class AFuture;
    friend class AThreadPool;
    friend struct aui::impl::future::Access;

    Inner* mInner = nullptr;

    struct Adopt {};
    AFuture(Inner* inner, Adopt) noexcept : mInner(inner) {}

    void reset() noexcept {
        if (auto inner = std::exchange(mInner, nullptr)) {
            inner->releaseFuture();
        }
    }

    [[nodiscard]]
    Inner& inner_() const noexcept {
        AUI_ASSERTX(mInner != nullptr, "the future is null");
        return *mInner;
    }

public:
    using Value = T;
    using Task = std::function<T()>;
#if AUI_COROUTINES
    using promise_type = aui::impl::future::CoPromiseType<T>;
#endif

    /**
     * @brief The producer side of an AFuture.
     * @details
     * Obtained from [AFuture::create]. Supplies the result with `supplyValue` or `supplyException`. Copies refer to
     * the same future. When the last copy is destroyed before a result is supplied, the future fails with
     * [AFutureAbandonedException].
     *
     * A resolver also tells the producer whether anybody still waits: see `isCancelled` and `onCancel`, the
     * counterparts of `AbortSignal` in JavaScript and `CancellationToken` in C#.
     */
    class Resolver {
    public:
        Resolver(const Resolver& rhs) noexcept : mInner(rhs.mInner) {
            if (mInner) {
                mInner->acquireResolver();
            }
        }
        Resolver(Resolver&& rhs) noexcept : mInner(std::exchange(rhs.mInner, nullptr)) {}
        Resolver& operator=(const Resolver& rhs) noexcept {
            if (this != &rhs) {
                Resolver copy(rhs);
                std::swap(mInner, copy.mInner);
            }
            return *this;
        }
        Resolver& operator=(Resolver&& rhs) noexcept {
            if (this != &rhs) {
                reset();
                mInner = std::exchange(rhs.mInner, nullptr);
            }
            return *this;
        }
        ~Resolver() { reset(); }

        /**
         * @brief Completes the future with a value.
         * @details
         * Calls the `onValue` callbacks on the current thread. Does nothing if the future is complete or cancelled.
         */
        void supplyValue(NonVoid v) const noexcept
            requires(!isVoid)
        {
            inner_().supplyValue(std::move(v));
        }

        /**
         * @brief Completes the future.
         * @details
         * Calls the `onValue` callbacks on the current thread. Does nothing if the future is complete or cancelled.
         */
        void supplyValue() const noexcept
            requires isVoid
        {
            inner_().supplyValue();
        }

        /**
         * @brief Completes the future with an exception, std::current_exception() by default.
         * @details
         * Calls the `onException` callbacks on the current thread. Does nothing if the future is complete or
         * cancelled.
         */
        void supplyException(std::exception_ptr causedBy = std::current_exception()) const noexcept {
            inner_().supplyException(std::move(causedBy));
        }

        /**
         * @return true if the value or the exception is supplied.
         */
        [[nodiscard]]
        bool hasResult() const noexcept {
            return inner_().hasResult();
        }

        /**
         * @return true if nobody waits for the result anymore, so the work can be stopped.
         */
        [[nodiscard]]
        bool isCancelled() const noexcept {
            return inner_().isCancelled();
        }

        /**
         * @brief Calls the callback when the future is cancelled, or right away if it is cancelled already.
         * @details
         * The callback is dropped without a call if the future completes. Use it to stop the work: close a socket,
         * abort a request, remove a timer.
         */
        template <aui::invocable Callback>
        void onCancel(Callback&& callback) const {
            inner_().subscribe(
                aui::impl::future::NoCallback {}, aui::impl::future::NoCallback {},
                std::forward<Callback>(callback));
        }

        /**
         * @brief Completes the future the way `other` completes, like `resolve(otherPromise)` in JavaScript.
         * @details
         * The future keeps `other` alive while anybody waits for it, so cancelling the future cancels `other`. If
         * `other` is cancelled, the future fails with [AFutureAbandonedException].
         */
        void supplyFrom(AFuture other) const {
            using namespace aui::impl::future;
            other.inner_().subscribe(
                [resolver = *this](const auto&... value) { resolver.supplyValue(value...); },
                [resolver = *this](const AException& e) { resolver.supplyException(causeOf(e)); }, NoCallback {});
            onCancel([other = std::move(other)] {});
        }

    private:
        friend class AFuture;
        friend class AThreadPool;
        friend struct aui::impl::future::Access;

        Inner* mInner = nullptr;

        Resolver(Inner* inner, Adopt) noexcept : mInner(inner) {}

        void reset() noexcept {
            if (auto inner = std::exchange(mInner, nullptr)) {
                inner->releaseResolver();
            }
        }

        [[nodiscard]]
        Inner& inner_() const noexcept {
            AUI_ASSERTX(mInner != nullptr, "the resolver is moved out");
            return *mInner;
        }
    };

    AFuture() = delete; // use create instead

    /**
     * @brief A null future, a placeholder to assign a real one to later.
     */
    AFuture(std::nullptr_t) noexcept {}

    /**
     * @deprecated use AFuture::fromValue.
     */
    [[deprecated("use AFuture::fromValue")]]
    explicit AFuture(NonVoid immediateValue)
        requires(!isVoid)
      : mInner(new Inner(Inner::FUTURE)) {
        mInner->supplyValue(std::move(immediateValue));
    }

    AFuture(const AFuture& rhs) noexcept : mInner(rhs.mInner) {
        if (mInner) {
            mInner->acquireFuture();
        }
    }
    AFuture(AFuture&& rhs) noexcept : mInner(std::exchange(rhs.mInner, nullptr)) {}

    AFuture& operator=(const AFuture& rhs) noexcept {
        if (this != &rhs) {
            AFuture copy(rhs);
            std::swap(mInner, copy.mInner);
        }
        return *this;
    }

    AFuture& operator=(AFuture&& rhs) noexcept {
        if (this != &rhs) {
            reset();
            mInner = std::exchange(rhs.mInner, nullptr);
        }
        return *this;
    }

    AFuture& operator=(std::nullptr_t) noexcept {
        reset();
        return *this;
    }

    ~AFuture() { reset(); }

    /**
     * @brief Makes a future completed by the code of `body`.
     * @param body called right away with a [AFuture::Resolver]; it may pass the resolver anywhere: to another thread,
     *        to a callback of an older API.
     * @details
     * The counterpart of `new Promise((resolve, reject) => ...)` in JavaScript, `suspendCancellableCoroutine` in
     * Kotlin and `TaskCompletionSource` in C#.
     *
     * If `body` throws, the future fails with that exception. If the resolver is lost before supplying a result, the
     * future fails with [AFutureAbandonedException].
     *
     * ```cpp
     * AFuture<int> answer() {
     *   return AFuture<int>::create([](AFuture<int>::Resolver resolver) {
     *     AThread::main()->enqueue([resolver] { resolver.supplyValue(42); });
     *   });
     * }
     * ```
     */
    template <aui::invocable<Resolver> Body>
    [[nodiscard]]
    static AFuture create(Body&& body) {
        auto inner = new Inner(Inner::FUTURE + 2 * Inner::RESOLVER);
        AFuture future(inner, Adopt {});
        Resolver guard(inner, Adopt {});   // so an exception from body wins over abandonment
        try {
            std::invoke(std::forward<Body>(body), Resolver(inner, Adopt {}));
        } catch (...) {
            guard.supplyException();
        }
        return future;
    }

    /**
     * @brief Makes a future that has the value already, like `Promise.resolve` in JavaScript.
     */
    template <typename... Args>
    [[nodiscard]]
    static AFuture fromValue(Args&&... args)
        requires(isVoid ? sizeof...(Args) == 0 : std::is_constructible_v<NonVoid, Args...>)
    {
        auto inner = new Inner(Inner::FUTURE);
        inner->supplyValue(std::forward<Args>(args)...);
        return AFuture(inner, Adopt {});
    }

    /**
     * @brief Makes a future that has failed already, like `Promise.reject` in JavaScript.
     */
    [[nodiscard]]
    static AFuture fromException(std::exception_ptr causedBy = std::current_exception()) {
        auto inner = new Inner(Inner::FUTURE);
        inner->supplyException(std::move(causedBy));
        return AFuture(inner, Adopt {});
    }

    /**
     * @brief Completes when every future has a value; fails as soon as one of them fails.
     * @return the values, in the order of the arguments; std::monostate in place of `void`.
     * @details
     * The counterpart of `Promise.all` in JavaScript, `awaitAll` in Kotlin and `Task.WhenAll` in C#.
     *
     * The returned future holds the futures given to it. When it completes or fails, it drops them, so the ones still
     * running are cancelled unless somebody else holds them. When it is cancelled, they are dropped as well.
     *
     * ```cpp
     * auto [user, avatar] = co_await AFuture<>::all(loadUser(id), loadAvatar(id));
     * ```
     */
    template <typename... Types>
    [[nodiscard]]
    static AFuture<std::tuple<aui::impl::future::Storage<Types>...>> all(AFuture<Types>... futures);

    /**
     * @brief Completes as soon as one of the futures completes or fails, with its value or exception.
     * @return the value of the first future to complete; its index in the variant is its position in the arguments.
     * @details
     * The counterpart of `Promise.race` in JavaScript, `select` in Kotlin and `Task.WhenAny` in C#. The other futures
     * are dropped, so they are cancelled unless somebody else holds them.
     *
     * ```cpp
     * auto result = co_await AFuture<>::any(fromCache(key), fromNetwork(key));
     * ```
     */
    template <typename... Types>
    [[nodiscard]]
    static AFuture<std::variant<aui::impl::future::Storage<Types>...>> any(AFuture<Types>... futures);

    /**
     * @brief A future that completes with this one, or fails with [ATimeoutException] as the cause if this one does
     * not complete in time.
     * @details
     * Works like `any(*this, AThread::asyncSleep(duration))`, keeping the type of the value. On timeout, this future
     * is cancelled, so its producer can abort the operation (see `Resolver::onCancel`). The returned future keeps this
     * one alive; cancelling or destroying the returned future cancels this one too.
     *
     * ```cpp
     * try {
     *     auto reply = co_await sendRequest().withTimeout(10s);
     * } catch (const AException& e) {
     *     // timed out, or sendRequest failed
     * }
     * ```
     */
    [[nodiscard]]
    AFuture withTimeout(std::chrono::milliseconds duration) const;

    /**
     * @brief Keeps the operation running when every AFuture of it is destroyed, until it completes.
     * @details
     * "Fire and forget": by default, destroying the last AFuture cancels the operation. Use keepAlive for an
     * operation whose result nobody waits for, but which has to finish: sending a log, saving a file. The callbacks
     * subscribed with `onValue`/`onException` are still called.
     *
     * ```cpp
     * sendAnalytics(event).onException([](const AException& e) { ALogger::warn("Analytics") << e; }).keepAlive();
     * ```
     */
    void keepAlive() const noexcept { inner_().keepAlive(); }

    /**
     * @brief Internal state; an identity of the future.
     */
    [[nodiscard]]
    const void* inner() const noexcept {
        return mInner;
    }

    /**
     * @return true if call to wait() function would cause thread block.
     */
    [[nodiscard]]
    bool isWaitNeeded() const noexcept {
        return !inner_().isSettled();
    }

    /**
     * @return true if the value or the exception was received.
     */
    [[nodiscard]]
    bool hasResult() const noexcept {
        return inner_().hasResult();
    }

    /**
     * @return true if the exception was received.
     */
    [[nodiscard]]
    bool hasException() const noexcept {
        return inner_().hasException();
    }

    /**
     * @return true if the asynchronous operation was successfully completed and supplied a value, which can be
     * obtained without waiting.
     */
    [[nodiscard]]
    bool hasValue() const noexcept {
        return inner_().hasValue();
    }

    /**
     * @return true if the future was cancelled before it got a result.
     */
    [[nodiscard]]
    bool isCancelled() const noexcept {
        return inner_().isCancelled();
    }

    /**
     * @brief Cancels the operation.
     * @details
     * When cancel() is called, there are 3 possible cases:
     * 1. AFuture's task is not taken by AThreadPool. The task is simply removed from the AThreadPool queue.
     * 2. AFuture's task is being executed. The task's thread is
     *    [requested for interrupt](AAbstractThread::interrupt()).
     * 3. AFuture's task is already completed. cancel() does nothing.
     *
     * The `onValue`/`onException` callbacks are dropped without a call; the producer is notified through
     * `Resolver::onCancel`. Destroying the last AFuture of the operation calls cancel().
     */
    void cancel() const noexcept { inner_().cancel(); }

    /**
     * @brief Sleeps if the result is not currently available.
     * @details
     * The task will be executed inside wait() function if the threadpool have not taken the task to execute
     * yet. This behaviour can be disabled by <code>AFutureWait::JUST_WAIT</code> flag.
     *
     * Exceptions captured from AFuture's task are not propagated. This function contains
     * `AThread::interruptionPoint()`: if `wait`'s caller's thread is interrupted, this function throws an
     * `AThread::Interrupted` exception.
     */
    void wait(AFutureWait flags = AFutureWait::DEFAULT) const {
        inner_().wait(flags);
        inner_().checkForSelfWait();
    }

    /**
     * @brief Returns the result. Sleeps if it is not currently available.
     * <dl>
     *   <dt><b>Sneaky exceptions</b></dt>
     *   <dd><code>AInvoсationTargetException</code> thrown if invocation target has thrown an exception or the future
     *   is cancelled.</dd>
     * </dl>
     */
    typename aui::impl::future::ReturnType<T>::type get(AFutureWait flags = AFutureWait::DEFAULT) const {
        AThread::interruptionPoint();
        auto& inner = inner_();
        inner.wait(flags);
        switch (inner.state.load(std::memory_order_acquire)) {
            case Inner::State::VALUE:
                if constexpr (!isVoid) {
                    return *inner.value;
                } else {
                    return;
                }
            case Inner::State::EXCEPTION:
                throw *inner.exception;
            case Inner::State::CANCELLED:
                throw AInvocationTargetException("Future is cancelled", nullptr);
            case Inner::State::PENDING:
                break;
        }
        inner.checkForSelfWait();
        throw AInvocationTargetException("Future execution interrupted", nullptr);
    }

    /**
     * @copybrief get
     */
    typename aui::impl::future::ReturnType<T>::type operator*() const { return get(); }

    /**
     * @copybrief get
     */
    NonVoid* operator->() const
        requires(!isVoid)
    {
        return &get();
    }

    /**
     * @brief Calls the callback with the value when it is supplied.
     * @details
     * The counterpart of `then` in JavaScript. The callback is called on the thread that supplies the value, or right
     * away if the value is already there. It is not called if the future fails or is cancelled, including by the
     * destruction of its last AFuture. To keep the future alive, put it into an AAsyncHolder of your window or object:
     *
     * ```cpp
     * mAsync << functionReturningFuture().onValue(...); // or onException
     * ```
     */
    template <aui::impl::future::OnValueCallback<T> Callback>
    const AFuture& onValue(Callback&& callback) const& {
        inner_().subscribe(
            std::forward<Callback>(callback), aui::impl::future::NoCallback {}, aui::impl::future::NoCallback {});
        return *this;
    }

    /**
     * @copybrief onValue
     */
    template <aui::impl::future::OnValueCallback<T> Callback>
    [[nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onValue(Callback&& callback) && {
        onValue(std::forward<Callback>(callback));
        return std::move(*this);
    }

    /**
     * @brief Calls the callback with the exception when the future fails.
     * @details
     * The counterpart of `catch` in JavaScript. The callback gets AInvocationTargetException; its chain of causes
     * leads to the original exception. Like `onValue`, it is not called if the future is cancelled.
     */
    template <aui::invocable<const AException&> Callback>
    const AFuture& onException(Callback&& callback) const& {
        inner_().subscribe(
            aui::impl::future::NoCallback {}, std::forward<Callback>(callback), aui::impl::future::NoCallback {});
        return *this;
    }

    /**
     * @copybrief onException
     */
    template <aui::invocable<const AException&> Callback>
    [[nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onException(Callback&& callback) && {
        onException(std::forward<Callback>(callback));
        return std::move(*this);
    }

    /**
     * @brief Calls the callback when the future completes or fails, like `finally` in JavaScript.
     */
    template <aui::invocable Callback>
    const AFuture& onFinally(Callback&& callback) const& {
        inner_().subscribe(
            [callback](const auto&...) mutable { callback(); },
            [callback](const AException&) mutable { callback(); }, aui::impl::future::NoCallback {});
        return *this;
    }

    /**
     * @copybrief onFinally
     */
    template <aui::invocable Callback>
    [[nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onFinally(Callback&& callback) && {
        onFinally(std::forward<Callback>(callback));
        return std::move(*this);
    }

    /**
     * @brief Calls the callback when the future is cancelled before it has a result.
     * @details
     * Cancellation happens by `cancel()` or when the last AFuture is destroyed. The callback is called on the thread
     * that cancels, right away if the future is cancelled already, and never if the future completes. It must not hold
     * this future, otherwise the future can't be cancelled by destruction.
     *
     * A producer that holds the [AFuture::Resolver] uses `Resolver::onCancel` instead.
     */
    template <aui::invocable Callback>
    const AFuture& onCancel(Callback&& callback) const& {
        inner_().subscribe(
            aui::impl::future::NoCallback {}, aui::impl::future::NoCallback {}, std::forward<Callback>(callback));
        return *this;
    }

    /**
     * @copybrief onCancel
     */
    template <aui::invocable Callback>
    [[nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onCancel(Callback&& callback) && {
        onCancel(std::forward<Callback>(callback));
        return std::move(*this);
    }

    /**
     * @deprecated use onValue.
     */
    template <aui::impl::future::OnValueCallback<T> Callback>
    [[deprecated("use onValue")]]
    const AFuture& onSuccess(Callback&& callback) const& {
        return onValue(std::forward<Callback>(callback));
    }

    /**
     * @deprecated use onValue.
     */
    template <aui::impl::future::OnValueCallback<T> Callback>
    [[deprecated("use onValue"), nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onSuccess(Callback&& callback) && {
        return std::move(*this).onValue(std::forward<Callback>(callback));
    }

    /**
     * @deprecated use onException.
     */
    template <aui::invocable<const AException&> Callback>
    [[deprecated("use onException")]]
    const AFuture& onError(Callback&& callback) const& {
        return onException(std::forward<Callback>(callback));
    }

    /**
     * @deprecated use onException.
     */
    template <aui::invocable<const AException&> Callback>
    [[deprecated("use onException"), nodiscard(AUI_IMPL_FUTURE_DISCARDED)]]
    AFuture onError(Callback&& callback) && {
        return std::move(*this).onException(std::forward<Callback>(callback));
    }

    /**
     * @brief Maps the value of this future to the value of another future.
     * @details
     * The counterpart of `then` returning a value in JavaScript. If the callback throws, the returned future fails
     * with the exception. If this future fails, the returned one fails as well. The returned future holds this one:
     * dropping the returned future drops this one.
     */
    template <aui::impl::future::OnValueCallback<T> Callback>
    [[nodiscard]]
    auto map(Callback&& callback) const {
        using R = typename std::conditional_t<
            isVoid, std::invoke_result<Callback>, std::invoke_result<Callback, const NonVoid&>>::type;
        return AFuture<R>::create(
            [&](typename AFuture<R>::Resolver resolver) {
                resolver.onCancel([source = *this] {});
                inner_().subscribe(
                    [resolver, callback = std::forward<Callback>(callback)](const auto&... value) mutable {
                        try {
                            if constexpr (std::is_void_v<R>) {
                                callback(value...);
                                resolver.supplyValue();
                            } else {
                                resolver.supplyValue(callback(value...));
                            }
                        } catch (...) {
                            resolver.supplyException();
                        }
                    },
                    [resolver](const AException& e) { resolver.supplyException(aui::impl::future::causeOf(e)); },
                    [resolver] { resolver.supplyException(aui::impl::future::cancelledException()); });
            });
    }

    [[nodiscard]]
    bool operator==(std::nullptr_t) const noexcept {
        return mInner == nullptr;
    }

    [[nodiscard]]
    bool operator==(const AFuture& r) const noexcept {
        return mInner == r.mInner;
    }

private:
    /**
     * @brief A future of a task that runs on the first call of `Access::execute` or on wait().
     */
    static std::pair<AFuture, Resolver> deferred(Task task) {
        // a future, the task (a producer until it runs) and the resolver for the executor
        auto inner = new Inner(Inner::FUTURE + 2 * Inner::RESOLVER, std::move(task));
        return { AFuture(inner, Adopt {}), Resolver(inner, Adopt {}) };
    }

    static bool execute(const Resolver& resolver) { return resolver.inner_().tryExecute(); }
};

namespace aui::impl::future {

/**
 * @brief Internal access to AFuture for AAsyncHolder, AComplexFutureOperation and coroutines.
 */
struct Access {
    template <typename T>
    static std::pair<AFuture<T>, typename AFuture<T>::Resolver> pending() {
        auto inner = new Inner<T>(Inner<T>::FUTURE + Inner<T>::RESOLVER);
        return { AFuture<T>(inner, typename AFuture<T>::Adopt {}),
                 typename AFuture<T>::Resolver(inner, typename AFuture<T>::Adopt {}) };
    }

    /**
     * @brief Subscribes for the value, the exception and the cancellation at once. Pass NoCallback to skip one.
     */
    template <typename T, typename V, typename E, typename C>
    static void subscribe(const AFuture<T>& future, V&& v, E&& e, C&& c) {
        future.inner_().subscribe(std::forward<V>(v), std::forward<E>(e), std::forward<C>(c));
    }
};

}   // namespace aui::impl::future

template <aui::invocable Fn>
auto AAbstractThread::async(Fn fn) -> AFuture<typename aui::impl::future::Unwrap<std::invoke_result_t<Fn&>>::type> {
    using Result = std::invoke_result_t<Fn&>;
    using Future = AFuture<typename aui::impl::future::Unwrap<Result>::type>;
    return Future::create([&](typename Future::Resolver resolver) {
        auto run = [fn = std::move(fn), resolver = std::move(resolver)]() mutable {
            if (resolver.isCancelled()) {
                return;
            }
            try {
                if constexpr (std::is_void_v<Result>) {
                    fn();
                    resolver.supplyValue();
                } else if constexpr (std::is_same_v<Result, Future>) {
                    resolver.supplyFrom(fn());
                } else {
                    resolver.supplyValue(fn());
                }
            } catch (...) {
                resolver.supplyException();
            }
        };
        if constexpr (std::is_copy_constructible_v<decltype(run)>) {
            enqueue(std::move(run));
        } else {
            // std::function requires a copyable callable
            enqueue([shared = std::make_shared<decltype(run)>(std::move(run))] { (*shared)(); });
        }
    });
}

template <typename T>
template <typename... Types>
AFuture<std::tuple<aui::impl::future::Storage<Types>...>> AFuture<T>::all(AFuture<Types>... futures) {
    using namespace aui::impl::future;
    using Result = std::tuple<Storage<Types>...>;
    using Resolver = typename AFuture<Result>::Resolver;
    return AFuture<Result>::create([&](Resolver resolver) {
        struct State {
            Resolver resolver;
            std::tuple<AOptional<Storage<Types>>...> values;
            std::atomic_size_t remaining = sizeof...(Types);
            ASpinlockMutex mutex;
            AOptional<std::tuple<AFuture<Types>...>> futures;

            explicit State(Resolver resolver) : resolver(std::move(resolver)) {}

            void dropFutures() {
                std::unique_lock lock(mutex);
                auto local = std::move(futures);
                futures.reset();
                lock.unlock();
            }
        };
        if constexpr (sizeof...(Types) == 0) {
            resolver.supplyValue(Result {});
        } else {
            auto state = _new<State>(resolver);
            state->futures.emplace(futures...);
            resolver.onCancel([state] { state->dropFutures(); });
            [&]<std::size_t... I>(std::index_sequence<I...>) {
                (Access::subscribe(
                     futures,
                     [state](const auto&... value) {
                         if constexpr (sizeof...(value) == 0) {
                             std::get<I>(state->values).emplace();
                         } else {
                             std::get<I>(state->values).emplace(value...);
                         }
                         if (state->remaining.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                             state->resolver.supplyValue(std::apply(
                                 [](auto&... v) { return Result { std::move(*v)... }; }, state->values));
                             state->dropFutures();
                         }
                     },
                     [state](const AException& e) {
                         state->resolver.supplyException(causeOf(e));
                         state->dropFutures();
                     },
                     [state] {
                         state->resolver.supplyException(cancelledException());
                         state->dropFutures();
                     }),
                 ...);
            }(std::index_sequence_for<Types...> {});
        }
    });
}

template <typename T>
template <typename... Types>
AFuture<std::variant<aui::impl::future::Storage<Types>...>> AFuture<T>::any(AFuture<Types>... futures) {
    static_assert(sizeof...(Types) > 0, "any of nothing never completes");
    using namespace aui::impl::future;
    using Result = std::variant<Storage<Types>...>;
    using Resolver = typename AFuture<Result>::Resolver;
    return AFuture<Result>::create([&](Resolver resolver) {
        struct State {
            Resolver resolver;
            ASpinlockMutex mutex;
            AOptional<std::tuple<AFuture<Types>...>> futures;

            explicit State(Resolver resolver) : resolver(std::move(resolver)) {}

            void dropFutures() {
                std::unique_lock lock(mutex);
                auto local = std::move(futures);
                futures.reset();
                lock.unlock();
            }
        };
        auto state = _new<State>(resolver);
        state->futures.emplace(futures...);
        resolver.onCancel([state] { state->dropFutures(); });
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (Access::subscribe(
                 futures,
                 [state](const auto&... value) {
                     if constexpr (sizeof...(value) == 0) {
                         state->resolver.supplyValue(Result(std::in_place_index<I>));
                     } else {
                         state->resolver.supplyValue(Result(std::in_place_index<I>, value...));
                     }
                     state->dropFutures();
                 },
                 [state](const AException& e) {
                     state->resolver.supplyException(causeOf(e));
                     state->dropFutures();
                 },
                 [state] {
                     state->resolver.supplyException(cancelledException());
                     state->dropFutures();
                 }),
             ...);
        }(std::index_sequence_for<Types...> {});
    });
}

template <typename T>
AFuture<T> AFuture<T>::withTimeout(std::chrono::milliseconds duration) const {
    using namespace aui::impl::future;
    return create([&](Resolver resolver) {
        auto timer = AThread::asyncSleep(duration);
        inner_().subscribe(
            [resolver](const auto&... value) {
                if constexpr (std::is_void_v<T> || std::is_copy_constructible_v<T>) {
                    resolver.supplyValue(value...);
                } else {
                    // a move-only value is moved out, as co_await does
                    resolver.supplyValue(std::move(const_cast<T&>(value))...);
                }
            },
            [resolver](const AException& e) { resolver.supplyException(causeOf(e)); },
            [resolver] { resolver.supplyException(cancelledException()); });
        timer.inner_().subscribe(
            [resolver, source = *this, duration] {
                resolver.supplyException(std::make_exception_ptr(
                    ATimeoutException("timed out after " + AString::number(duration.count()) + "ms")));
                // let the producer abort the operation
                source.cancel();
            },
            NoCallback {}, NoCallback {});
        resolver.onCancel([source = *this, timer = std::move(timer)] { source.cancel(); });
    });
}

template <typename T>
void aui::impl::future::Inner<T>::wait(ABitField<AFutureWait> flags) {
    if (isSettled()) {
        return;   // cheap check
    }
    try {
        if (flags & AFutureWait::ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP) {
            // the task may be not picked up by the threadpool yet; execute it here
            if (tryExecute()) {
                return;
            }
        }

        if (flags & AFutureWait::ALLOW_STACKFUL_COROUTINES) {
            if (auto threadPoolWorker = _cast<AThreadPool::Worker>(AThread::current())) {
                if (isSettled()) {
                    return;
                }
                checkForSelfWait();
                auto wakeUp = [threadPoolWorker](const auto&...) { threadPoolWorker->threadPool().wakeUpAll(); };
                subscribe(wakeUp, wakeUp, wakeUp);
                threadPoolWorker->loop([&] { return !isSettled(); });
                return;
            }
        }
        std::unique_lock lock(mutex);
        while (!isSettled()) {
            if (executing && thread == AThread::current()) [[unlikely]] {
                // self wait?
                return;
            }
            if (!cv) {
                cv = std::make_unique<AConditionVariable>();
            }
            cv->wait(lock);
        }
    } catch (const AThread::Interrupted& e) {
        e.needRethrow();
    }
    AThread::interruptionPoint();
}

#undef AUI_IMPL_FUTURE_DISCARDED

#if AUI_COROUTINES
#include "AFutureCpp20Coro.h"
#endif
