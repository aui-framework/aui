#pragma once

#include "AFuture.h"

namespace aui::impl::future {

/**
 * @brief Concept for types that satisfy the C++20 Awaiter requirements.
 * @details
 * An Awaiter must provide three methods:
 * - await_ready() returning a boolean
 * - await_suspend(std::coroutine_handle<>)
 * - await_resume()
 *
 * This is used to constrain await_transform overloads.
 */
template <typename T>
concept Cpp20Awaiter = requires(T &t) {
    { t.await_ready() } -> ::aui::convertible_to<bool>;
    { t.await_suspend(std::coroutine_handle<>{}) };
    { t.await_resume() };
};

/**
 * @brief Concept for types that satisfy the C++20 Promise requirements.
 * @details
 * A Promise must provide:
 * - initial_suspend() returning an Awaiter
 * - final_suspend() returning an Awaiter
 * - get_return_object()
 */
template <typename T>
concept Cpp20Promise = requires(T &t) {
    { t.initial_suspend() } -> Cpp20Awaiter;
    { t.final_suspend() } -> Cpp20Awaiter;
    { t.get_return_object() };
};

/**
 * @brief Concept for types that have a nested promise_type satisfying Cpp20Promise.
 * @details
 * This is used to detect types that can be directly co_await ed (like AFuture<T>),
 * which have an associated promise_type that meets the C++20 coroutine requirements.
 */
template <typename T>
concept Cpp20HasPromiseType = requires { typename T::promise_type; } && Cpp20Promise<typename T::promise_type>;

/**
 * @brief Promise type base for C++20 coroutines returning AFuture<T>.
 * @tparam T The value type of the AFuture<T> returned by the coroutine.
 * @details
 * CoPromiseTypeBase serves as the coroutine promise object for functions that return
 * AFuture<T>. It bridges the C++20 coroutine framework with AUI's AFuture mechanism,
 * allowing coroutines to seamlessly interoperate with the existing asynchronous future
 * infrastructure.
 *
 * The promise object is constructed when a coroutine function is called and manages the
 * coroutine's lifecycle, result delivery, exception propagation, and co_await integration.
 *
 */
template <typename T>
struct CoPromiseTypeBase {
    /**
     * @brief The future returned to the caller of the coroutine and the resolver the coroutine completes it with.
     * @details
     * If the coroutine frame is destroyed without a result (for example, a pending resumption is dropped), the
     * resolver fails the future with AFutureAbandonedException.
     */
    std::pair<AFuture<T>, typename AFuture<T>::Resolver> handles = Access::pending<T>();

    /**
     * @brief Called immediately by the compiler after the coroutine starts.
     * @return std::suspend_never, meaning the coroutine starts executing right away (not lazily).
     */
    auto initial_suspend() noexcept { return std::suspend_never {}; }

    /**
     * @brief Called by the compiler when the coroutine reaches its final suspension point.
     * @return std::suspend_never, meaning the coroutine frame is destroyed immediately after completion.
     */
    auto final_suspend() noexcept { return std::suspend_never {}; }

    /**
     * @brief Passes an exception escaped from the coroutine body to the future. Ignored if the future is cancelled.
     */
    void unhandled_exception() noexcept { handles.second.supplyException(); }

    /**
     * @brief Called by the compiler to obtain the return value for the caller of the coroutine.
     */
    AFuture<T> get_return_object() noexcept {
        AUI_ASSERT(handles.first != nullptr);
        return std::move(handles.first);
    }

    /**
     * @brief Overload of await_transform for types that are already Cpp20Awaiter.
     * @details
     * Before resuming, checks whether the future of this coroutine is cancelled. If so, throws AThread::Interrupted
     * to unwind the coroutine instead of continuing work nobody waits for.
     */
    template <Cpp20Awaiter NestedAwaiter>
    auto await_transform(NestedAwaiter&& nestedAwaiter) {
        struct Awaiter {
            NestedAwaiter nestedAwaiter;
            const typename AFuture<T>::Resolver& resolver;

            bool await_ready() noexcept { return nestedAwaiter.await_ready(); }

            auto await_resume() {
                if (resolver.isCancelled()) {
                    throw AThread::Interrupted {};
                }
                return nestedAwaiter.await_resume();
            }

            auto await_suspend(std::coroutine_handle<CoPromiseType<T>> ourHandle) {
                return nestedAwaiter.await_suspend(std::move(ourHandle));
            }
        };
        return Awaiter { .nestedAwaiter = std::forward<NestedAwaiter>(nestedAwaiter), .resolver = handles.second };
    }

    /**
     * @brief Overload of await_transform for types that have a promise_type (like AFuture<T>).
     */
    template <Cpp20HasPromiseType NestedAwaitable>
    auto await_transform(NestedAwaitable nestedAwaitable) {
        return await_transform(operator co_await(std::move(nestedAwaitable)));
    }

protected:
    template <typename... F>
    void resolve(F&&... v) noexcept {
        handles.second.supplyValue(std::forward<F>(v)...);
    }
};

/**
 * @brief Promise type of coroutines returning AFuture<T> with a value.
 */
template <typename T>
struct CoPromiseType : CoPromiseTypeBase<T> {
    template <typename F>
    void return_value(F&& v) noexcept {
        this->resolve(std::forward<F>(v));
    }
};

/**
 * @brief Promise type of coroutines returning AFuture<void>.
 */
template <>
struct CoPromiseType<void> : CoPromiseTypeBase<void> {
    void return_void() noexcept { resolve(); }
};
}   // namespace aui::impl::future

// Verify that AFuture<int> and AFuture<void> satisfy the Cpp20HasPromiseType concept.
static_assert(aui::impl::future::Cpp20HasPromiseType<AFuture<int>>);
static_assert(aui::impl::future::Cpp20HasPromiseType<AFuture<void>>);

/**
 * @brief Free operator co_await for AFuture<T>, enabling co_await on AFuture values.
 * @tparam T The value type of the future.
 * @param future The AFuture<T> to await.
 * @return An Awaiter that integrates with the coroutine suspension machinery.
 * @details
 * This is the primary mechanism by which AFuture<T> becomes awaitable. The returned Awaiter:
 *
 * - **await_ready**: Returns true if the future already has a result (no suspension).
 * - **await_resume**: Dereferences the future to obtain the value (or void).
 * - **await_suspend**: Registers callbacks for the value, the exception and the cancellation of the future. When the
 *   future completes (possibly on a different thread), the callback enqueues resumption
 *   onto the **caller's original thread** via AThread::current()->enqueue(). This ensures
 *   the coroutine resumes on the same thread that initiated the co_await, avoiding deep
 *   call stacks and inconsistent threading.
 */
template <typename T>
auto operator co_await(AFuture<T> future) {
    struct Awaiter {
        AFuture<T> future;

        bool await_ready() const noexcept { return future.hasResult() || future.isCancelled(); }

        auto await_resume() {
            if constexpr (std::is_same_v<T, void>) {
                *future;
            } else {
                return std::move(*future);
            }
        }

        void await_suspend(std::coroutine_handle<> handle) {
            // the callbacks are called by the thread that supplies the result, which might be a different thread.
            // if we keep this logic for coroutines, the callstack will grow very fast and inconsistent.
            // for co_await, let's keep the caller thread.
            // if user wants to magically switch threads using co_await, they'll figure out how to shoot their knee.
            // exactly one of the callbacks is called; a cancelled future resumes the coroutine as well, so that
            // await_resume throws instead of the coroutine hanging forever.
            auto callback = [handle = std::move(handle), callerThread = AThread::current()](const auto &...) {
                callerThread->enqueue([handle = std::move(handle)] { handle.resume(); });
            };
            aui::impl::future::Access::subscribe(future, callback, callback, callback);
        }
    };

    return Awaiter { std::move(future) };
}
