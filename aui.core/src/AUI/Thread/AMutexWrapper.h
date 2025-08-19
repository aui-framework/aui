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

#include <AUI/Traits/values.h>
#include <AUI/Traits/memory.h>

/**
 * @brief Wraps the object with a Lockable, providing exclusive access layer, i.e., for thread-safety.
 * @ingroup core
 * @tparam T The type of the value to be protected.
 * @tparam Lockable A lockable type which manages locking behaviour, i.e, AMutex.
 * @details
 * On debug builds, provides a runtime check for better diagnostics.
 *
 * Aka `boost::synchronized_value`.
 *
 * ## Implementing thread safety
 *
 * ```cpp
 * struct SharedResource {
 *   AString data;
 * };
 * AMutexWrapper<SharedResource, AMutex> sharedResource;
 * sharedResource->data = "test"; // bad, will crash
 *
 * ...
 * // thread 1
 * std::unique_lock lock(sharedResource); //
 * sharedResource->data = "hello";        // ok
 * ...
 *
 * // thread 2
 * std::unique_lock lock(sharedResource); //
 * sharedResource->data = "world";        // ok, will be done before or after "hello", but not simultaneously
 * ...
 * ```
 */
template<typename T, typename Lockable = AMutex>
class AMutexWrapper: public aui::noncopyable {
public:
    AMutexWrapper(T value = T()) noexcept: mValue(std::move(value)) {}


    void lock() {
        mMutex.lock();

#if AUI_DEBUG
        mOwnerThread = AThread::current();
#endif
    }

    AMutexWrapper& operator=(const T& rhs) {
        value() = rhs;
        return *this;
    }

    AMutexWrapper& operator=(T&& rhs) noexcept {
        value() = std::move(rhs);
        return *this;
    }

    template<typename U, std::enable_if_t<std::is_constructible_v<U, T>>* = 0>
    operator U() noexcept {
        return { value() };
    }

    void unlock() {
#if AUI_DEBUG
        mOwnerThread = nullptr;
#endif
        mMutex.unlock();
    }

    T& value() noexcept {
#if AUI_DEBUG
        AUI_ASSERTX(mOwnerThread == AThread::current(), "AMutexWrapper should be locked by this thread in order to get access to the underlying object");
#endif
        return mValue;
    }

    T* operator->() noexcept {
        return &value();
    }

private:
    T mValue;
    Lockable mMutex;

#if AUI_DEBUG
    _<AAbstractThread> mOwnerThread;
#endif
};
