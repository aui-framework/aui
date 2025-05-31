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
 * @brief A class that provides exclusive access to a resource in a single threaded context.
 * @details
 * Provides Rust-like resource owning semantics with runtime checks.
 *
 * ARuntimeBorrowChecker enforces to access the underlying object with either readScope or writeScope, both of these
 * modify the state of ARuntimeBorrowChecker to ensure borrowing semantics. It manages concurrent access (not
 * specifically multithread) to shared resources by enforcing the following rules:
 *
 * - Multiple simultaneous read operations are allowed
 * - Write operations are exclusive (no concurrent reads or writes)
 * - Access is managed through RAII-style scope objects
 */
template<typename T>
class ARuntimeBorrowChecker {
public:
    template <typename F>
    explicit ARuntimeBorrowChecker(F&& f) : mValue(std::forward<F>(f)) {}
    ARuntimeBorrowChecker() = default;
    ~ARuntimeBorrowChecker() {
        AUI_ASSERT(!is_locked());
    }

    struct Ref {
    public:
        explicit Ref(const ARuntimeBorrowChecker* underlying) {
            if (underlying->try_lock_shared()) {
                mUnderlying = underlying;
            }
        }

        [[nodiscard]]
        bool ownsLock() const noexcept {
            return mUnderlying != nullptr;
        }

        [[nodiscard]]
        const T& value() const noexcept {
            AUI_ASSERTX(ownsLock(), "Ref has failed to lock");
            return mUnderlying->mValue;
        }

        [[nodiscard]]
        operator bool() const noexcept {
            return ownsLock();
        }

        [[nodiscard]]
        const T* operator->() const noexcept {
            return &value();
        }

        ~Ref() {
            if (mUnderlying) {
                mUnderlying->unlock_shared();
            }
        }

    private:
        const ARuntimeBorrowChecker* mUnderlying = nullptr;
    };

    struct MutRef {
    public:
        explicit MutRef(ARuntimeBorrowChecker* underlying) {
            if (underlying->try_lock()) {
                mUnderlying = underlying;
            }
        }

        [[nodiscard]]
        bool ownsLock() const noexcept {
            return mUnderlying != nullptr;
        }

        [[nodiscard]]
        T& value() noexcept {
            AUI_ASSERTX(ownsLock(), "MutRef has failed to lock");
            return mUnderlying->mValue;
        }

        [[nodiscard]]
        const T& value() const noexcept {
            AUI_ASSERTX(ownsLock(), "MutRef has failed to lock");
            return mUnderlying->mValue;
        }

        [[nodiscard]]
        operator bool() const noexcept {
            return ownsLock();
        }

        [[nodiscard]]
        T* operator->() noexcept {
            return &value();
        }

        [[nodiscard]]
        const T* operator->() const noexcept {
            return &value();
        }

        ~MutRef() {
            if (mUnderlying) {
                mUnderlying->unlock();
            }
        }

    private:
        ARuntimeBorrowChecker* mUnderlying = nullptr;
    };

    Ref readScope() const {
        return Ref(this);
    }

    MutRef writeScope() {
        return MutRef(this);
    }

private:
    T mValue;

    /**
     * @brief Tries to acquire the lock without blocking for shared readonly access.
     * @return true if the lockable is successfully acquired, false otherwise.
     */
    [[nodiscard]]
    bool try_lock_shared() const noexcept {
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

    void unlock_shared() const noexcept {
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

    struct SharedLock {
        std::uint32_t count{};
    };
    struct ExclusiveLock {};
    mutable std::variant<std::monostate, SharedLock, ExclusiveLock> mState{};
};
