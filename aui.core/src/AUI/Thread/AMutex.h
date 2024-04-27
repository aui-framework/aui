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
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include "AUI/Performance/APerformanceSection.h"


namespace aui::detail {
    template<typename T>
    struct MutexExtras: T {
    public:
        void lock() {
            APerformanceSection section("Mutex", AColor::RED);
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
        APerformanceSection section("Mutex", AColor::RED);
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
        APerformanceSection section("Mutex", AColor::RED);
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

private:
    enum State { UNLOCKED, LOCKED };
    std::atomic<State> mState = UNLOCKED;
};