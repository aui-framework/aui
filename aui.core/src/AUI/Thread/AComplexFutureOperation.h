#pragma once

#include "AFuture.h"
#include "AAsyncHolder.h"

/**
 * @brief Helper class to construct AFuture values.
 * @ingroup core
 * @tparam T future type AFuture<T> (void is default)
 * @details
 * AComplexFutureOperation is used to handle results and errors of multiple sequential futures.
 *
 * AComplexFutureOperation is intended to construct a future with makeFuture(). makeFuture() must be called.
 *
 * If AComplexFutureOperation and its copies of the resolver are destroyed without a result, the future fails with
 * AFutureAbandonedException.
 *
 * @deprecated use a coroutine or AFuture::create.
 */
template<typename T>
class [[deprecated("use a coroutine or AFuture::create")]] AComplexFutureOperation {
public:
    AComplexFutureOperation(): mHandles(aui::impl::future::Access::pending<T>()) {}

    ~AComplexFutureOperation() {
        AUI_ASSERTX(mHandles.first == nullptr, "makeFuture() is not called. Please check docs");
    }

    /**
     * @brief Creates AFuture<T> object for this operation.
     * @details
     * AFuture<T> can be created only once.
     */
    [[nodiscard]]
    AFuture<T> makeFuture() noexcept {
        AUI_ASSERTX(mHandles.first != nullptr, "makeFuture() is called twice");
        return std::move(mHandles.first);
    }

    /**
     * @brief Holds the future and passes its exception to the future of this operation.
     */
    template<typename OtherT>
    AComplexFutureOperation& operator<<(AFuture<OtherT> rhs) {
        rhs.onException([resolver = mHandles.second](const AException& e) {
            resolver.supplyException(aui::impl::future::causeOf(e));
        });
        mAsyncHolder << std::move(rhs);
        return *this;
    }

    template<typename... Args>
    void supplyValue(Args&&... args) {
        mHandles.second.supplyValue(std::forward<Args>(args)...);
    }

    void supplyException(std::exception_ptr causedBy = std::current_exception()) {
        mHandles.second.supplyException(std::move(causedBy));
    }

private:
    std::pair<AFuture<T>, typename AFuture<T>::Resolver> mHandles;
    AAsyncHolder mAsyncHolder;
};
