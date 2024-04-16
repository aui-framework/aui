// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AFuture.h"
#include <any>
#include <list>


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
            l.cancel();
            l.wait();
            l = nullptr;
            lock.lock();
        }

        while (!mCustomTypeFutures.empty()) {
            auto l = std::move(mCustomTypeFutures.back());
            mCustomTypeFutures.pop_back();
            lock.unlock();
            l->cancelAndWait();
            l = nullptr;
            lock.lock();
        }
        mDead = true;
    }

    template<typename T>
    AAsyncHolder& operator<<(AFuture<T> future) {
        std::unique_lock lock(mSync);
        if constexpr (std::is_same_v<void, T>) {
            auto impl = future.inner().get();
            mFutureSet << std::move(future);
            lock.unlock();
            future.onSuccess([this, impl]() {
                std::unique_lock lock(mSync);
                mFutureSet.removeIf([&](const AFuture<>& f) {
                    return f.inner().get() == impl;
                });
            });
        } else {
            auto uniquePtr = std::make_unique<Future<T>>(future);
            auto it = mCustomTypeFutures.insert(mCustomTypeFutures.end(), std::move(uniquePtr));

            lock.unlock();

            future.onSuccess([this, it](const T& result) {
                AUI_ASSERTX(!mDead, "you have concurrency issues");
                std::unique_lock lock(mSync);
                AUI_ASSERT(!mCustomTypeFutures.empty());
                mCustomTypeFutures.erase(it);
            });
        }
        return *this;
    }

    [[nodiscard]]
    std::size_t size() const {
        std::unique_lock lock(mSync);
        return mFutureSet.size();
    }

    void waitForAll() {
        std::unique_lock lock(mSync);
        if (!mFutureSet.empty()) {
            waitAgain:
            auto futureSet = std::move(mFutureSet);
            lock.unlock();
            futureSet.waitForAll();
            lock.lock();
            if (mFutureSet.empty()) {
                mFutureSet = std::move(futureSet);
            } else {
                mFutureSet.insertAll(futureSet);
                goto waitAgain;
            }
        }

        while (!mCustomTypeFutures.empty()) {
            mCustomTypeFutures.front()->wait(lock);
        }
    }

private:
    struct IFuture {
        virtual ~IFuture() = default;
        virtual void get() = 0;
        virtual void wait(std::unique_lock<AMutex>& lock) = 0;
        virtual void cancelAndWait() = 0;
    };
    bool mDead = false;

    template<typename T>
    struct Future: IFuture {
        void get() override {
            mFuture.cancel();
            *mFuture;
        }
        void wait(std::unique_lock<AMutex>& lock) override {
            auto copy = mFuture;
            lock.unlock();
            copy.wait();
            lock.lock();
        }

        void cancelAndWait() override {
            mFuture.cancel();
            mFuture.wait();
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
