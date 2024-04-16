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
 * If AComplexFutureOperation get destroyed, it reports error to it's future if it does not have a result.
 * AComplexFutureOperation should be carried with AFuture::onSuccess callbacks of other futures or any other
 * callback-based structure, hence the destruction without result is treated as an error.
 */
template<typename T>
class AComplexFutureOperation {
public:
    class NoValueException: AException {
    public:
        NoValueException(): AException("no value") {}
    };

    AComplexFutureOperation(): mFutureWeakReference(mFutureStrongReference.inner().weak()) {}

    ~AComplexFutureOperation() {
        AUI_ASSERTX(mFutureStrongReference.inner() == nullptr, "makeFuture() is not called. Please check docs");
        if (auto f = mFutureWeakReference.lock()) {
            if (!(*f)->hasResult()) {
                try {
                    throw NoValueException();
                } catch (...) {
                    (*f)->reportException();
                }
            }
        }
    }

    /**
     * @brief Creates AFuture<T> object for this operation.
     * @details
     * AFuture<T> can be created only once.
     */
    [[nodiscard]]
    AFuture<T> makeFuture() const noexcept {
        return std::move(mFutureStrongReference);
    }

    /**
     * @brief Adds a direct dependency on other AFuture<T>.
     * @param rhs other future
     * @details
     * Stores other AFuture to internal AAsyncHolder storage. Passes rhs's onError calls to AComplexFutureOperation's
     * future, if it does not have result (!AFuture::hasResult()).
     */
    template<typename OtherT>
    AComplexFutureOperation& operator<<(AFuture<OtherT> rhs) {
        rhs.onError([weakRef = mFutureWeakReference](const AException& e) {
            if (auto f = weakRef.lock()) {
                if ((*f)->hasResult()) {
                    return;
                }
                try {
                    throw e;
                } catch (...) {
                    (*f)->reportException();
                }
            }
        });
        mAsyncHolder << std::move(rhs);
        return *this;
    }

    /**
     * @brief Pushes the result of operation to it's AFuture<T>.
     * @param value operation
     */
    void supplyValue(T value) {
        if (auto l = mFutureWeakReference.lock()) {
            auto& inner = *l;
            std::unique_lock lock(inner->mutex);
            inner->value = std::move(value);
            inner->cv.notify_all();
            inner->notifyOnSuccessCallback(lock);
        }
    }

    /**
     * @brief Pushes the result of operation to it's AFuture<T>.
     */
    void supplyException() {
        if (auto l = mFutureWeakReference.lock()) {
            auto& inner = *l;
            inner->reportException();
        }
    }

private:
    AFuture<T> mFutureStrongReference;
    AAsyncHolder mAsyncHolder;

    /*
     * Avoid holding a strong reference - we need to keep future cancellation on reference count exceeding
     * even while actual future execution.
     */
    _weak<typename AFuture<T>::Inner> mFutureWeakReference;
};