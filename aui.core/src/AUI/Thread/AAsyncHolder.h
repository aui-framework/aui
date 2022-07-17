#pragma once

#include "AFuture.h"
#include <any>


/**
 * @brief Holds a set of futures keeping them valid.
 * @ingroup core
 * @details
 * Unlike AFutureSet, AAsyncHolder is intended to hold void futures (AFuture<void>), however, non-void types can be also
 * supported (but with extra overhead). When AFuture's task is complete, the AFuture is removed from AAsyncHolder.
 *
 * Guarantees that held futures will never be executed or be during execution after AAsyncHolder is destroyed.
 *
 * Cancels all futures in destructor.
 */
class AAsyncHolder {
public:
    AAsyncHolder() = default;
    ~AAsyncHolder() {
        std::unique_lock lock(mSync);

        while (!mFutureSet.empty()) {
            auto l = std::move(mFutureSet.last());
            mFutureSet.pop_back();
            lock.unlock();
            l = nullptr;
            lock.lock();
        }

        while (!mCustomTypeFutures.empty()) {
            auto l = std::move(mCustomTypeFutures.back());
            mCustomTypeFutures.pop_back();
            lock.unlock();
            l = nullptr;
            lock.lock();
        }
    }

    template<typename T>
    AAsyncHolder& operator<<(AFuture<T> future) {
        std::unique_lock lock(mSync);
        auto uniquePtr = std::make_unique<Future<T>>(future);
        auto ptr = uniquePtr.get();
        mCustomTypeFutures.push_back(std::move(uniquePtr));
        future.onSuccess([this, ptr](const T& result) {
            std::unique_lock lock(mSync);
            mCustomTypeFutures.erase(std::remove_if(mCustomTypeFutures.begin(), mCustomTypeFutures.end(), [&](const auto& uniquePtr) {
                return uniquePtr.get() == ptr;
            }), mCustomTypeFutures.end());
        });
        return *this;
    }
    template<>
    AAsyncHolder& operator<<(AFuture<void> future) {
        std::unique_lock lock(mSync);
        auto impl = future.inner().get();
        future.onSuccess([this, impl]() {
            std::unique_lock lock(mSync);
            mFutureSet.removeIf([&](const AFuture<>& f) {
                return f.inner().get() == impl;
            });
        });
        mFutureSet << std::move(future);
        return *this;
    }

    [[nodiscard]]
    std::size_t size() const {
        std::unique_lock lock(mSync);
        return mFutureSet.size();
    }

private:
    struct IFuture {
        virtual ~IFuture() = default;
        virtual void get() = 0;
    };

    template<typename T>
    struct Future: IFuture {
        void get() override {
            mFuture.cancel();
            *mFuture;
        }
        ~Future() override = default;

        Future(AFuture<T> future) : mFuture(std::move(future)) {}

    private:
        AFuture<T> mFuture;
    };


    mutable AMutex mSync;
    AFutureSet<> mFutureSet;
    std::list<_unique<IFuture>> mCustomTypeFutures;
};