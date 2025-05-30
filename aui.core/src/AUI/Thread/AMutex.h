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
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <AUI/Util/Assert.h>
#include <utility>

namespace aui::detail {
    template<typename T>
    struct MutexExtras: T {
    public:
        void lock() {
            T::lock();
        }
    };
}

/**
 * @brief Basic syscall-based synchronization primitive.
 * @ingroup core
 */
struct AMutex: aui::detail::MutexExtras<std::mutex>{};

/**
 * @brief Like AMutex but can handle multiple locks for one thread (recursive).
 * @ingroup core
 * @details
 * @note Please note that the usage of recursive mutex may indicate that your code may have architectural issues related
 * to the concurrency (e.g., comodification of a container that is being foreach-looped). Use recursive mutex with care.
 */
struct ARecursiveMutex: aui::detail::MutexExtras<std::recursive_mutex>{};

/**
 * @brief Like AMutex but has shared lock type (in addition to basic lock which is unique locking) implementing RW
 * synchronization.
 * @ingroup core
 */
struct ASharedMutex: aui::detail::MutexExtras<std::shared_mutex> {
public:
    void lock_shared() {
        MutexExtras::lock_shared();
    }
};


/**
 * @brief Synchronization primitive that is implemented with atomic values instead of doing syscalls.
 * @details
 * In contrast to a regular mutex, threads will busy-wait (infinitely check for unlocked state) and waste CPU cycles
 * instead of yielding the CPU to another thread with a syscall.
 *
 * ASpinlockMutex may be faster than a regular mutex in some cases. Use benchmarks to compare. 
 */
class ASpinlockMutex {
public:
    void lock() {
        while (!try_lock()) {
            // busy-wait
        }
    }

    /**
     * @brief Tries to acquire the mutex without blocking.
     * @return true if the mutex is successfully acquired, false otherwise.
     */
    [[nodiscard]]
    bool try_lock() noexcept {
        return mState.exchange(LOCKED, std::memory_order_acquire) == UNLOCKED;
    }

    void unlock() noexcept {
        mState.store(UNLOCKED, std::memory_order_release);
    }

    [[nodiscard]]
    bool is_locked() const noexcept {
        return mState == LOCKED;
    }

private:
    enum State { UNLOCKED, LOCKED };
    std::atomic<State> mState = UNLOCKED;
};

/**
 * @brief A class that provides exclusive access to a resource in a single threaded context.
 * @details
 * Although this object implements mutex-like interface, it is intended to use in a single threaded context.
 * See @ref AExclusiveAccess.
 *
 * With AExclusiveAccessLockable, @ref AExclusiveAccess class can be reused in a single threaded context. This example
 * implements Rust-like behaviour that enforces either one or more readers or a single writer (but in runtime):
 *
 * @code{cpp}
 * // pseudocode
 * struct SharedResource {
 *   AVector<AString> users;
 * };
 * AExclusiveAccess<SharedResource, AMutex> sharedResource = { ... };
 *
 * void aReadOnlyHandler(const AString& user) {
 *     std::shared_lock lock(sharedResource);
 *     ...
 *     bool doSomeWork = sharedResource.contains(user);
 * }
 *
 * void aWriteHandler(const AString& user) {
 *     std::unique_lock lock(sharedResource); // exception: an attempt to acquire writing access
 *     sharedResource.remove(user);           // this would break all in progress for each loops anyway
 * }
 *
 * void broadcast(std::function<void(const AString&)> handler) {
 *     std::shared_lock lock(sharedResource);
 *     for (const auto& user : sharedResource->users) {
 *         handler(user); // we have a for each loop running in progress; an attempt to add/remove elements
 *                        // to users will break it
 *     }
 * }
 * @endcode
 */
class AExclusiveAccessLockable {
public:
    /**
     * @brief Tries to acquire the lock without blocking for shared readonly access.
     * @return true if the lockable is successfully acquired, false otherwise.
     */
    [[nodiscard]]
    bool try_lock_shared() noexcept {
        if (std::holds_alternative<std::monostate>(mState)) {
            mState = SharedLock { .count = 1 };
            return true;
        }
        if (auto shared = std::get_if<SharedLock>(&mState)) {
            shared->count++;
            return true;
        }
        return false;
    }

    /**
     * @brief Tries to acquire the lock without blocking for exclusive writeable access.
     * @return true if the lockable is successfully acquired, false otherwise.
     */
    [[nodiscard]]
    bool try_lock() noexcept {
        if (std::holds_alternative<std::monostate>(mState)) {
            mState = ExclusiveLock{};
            return true;
        }
        return false;
    }

    void unlock() noexcept {
#if AUI_DEBUG
        AUI_ASSERT(std::holds_alternative<ExclusiveLock>(mState));
#endif
        mState = std::monostate{};
    }

    void unlock_shared() noexcept {
#if AUI_DEBUG
        AUI_ASSERT(std::holds_alternative<SharedLock>(mState));
#endif
        if (auto shared = std::get_if<SharedLock>(&mState)) {
            if (--shared->count != 0) {
                return;
            }
        }
        mState = std::monostate{};
    }

    [[nodiscard]]
    bool is_locked() const noexcept {
        return !std::holds_alternative<std::monostate>(mState);
    }

private:
    struct SharedLock {
        std::uint32_t count{};
    };
    struct ExclusiveLock {};
    std::variant<std::monostate, SharedLock, ExclusiveLock> mState{};
};

/**
 * @brief Implements mutex interface but does nothing, useful for mocking a mutex.
 */
class AFakeMutex {
public:
    void lock() {}
    void try_lock() {}
    void unlock() {}
    void shared_lock() {}
    void try_shared_lock() {}
    void shared_unlock() {}
};
