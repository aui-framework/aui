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
 * Please note that the usage of recursive mutex may indicate that your code may have architectural issues related
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

    void unlock_shared() {
        MutexExtras::unlock_shared();
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
