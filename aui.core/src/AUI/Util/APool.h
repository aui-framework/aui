/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>
#include <AUI/Common/AQueue.h>
#include <AUI/Common/SharedPtrTypes.h>

template<typename T>
class APool {
private:
    std::function<T*()> mFactory;
    AQueue<T*> mQueue;
    std::shared_ptr<bool> mPoolAlive = std::make_shared<bool>(true);


    template <typename Ptr = _<T>>
    Ptr getImpl() noexcept {
        T* t;
        if (mQueue.empty()) {
            t = mFactory();
        } else {
            t = mQueue.front();
            mQueue.pop();
        }
        if constexpr (std::is_same_v<Ptr, _<T>>) {
            return aui::ptr::manage(t, APoolDeleter(this));
        } else {
            return std::unique_ptr<T, APoolDeleter>(t, APoolDeleter(this));
        }
    }

public:
    explicit APool(const std::function<T*()>& factory) noexcept : mFactory(factory) {}

    struct APoolDeleter {
        APool<T>* pool;
        std::shared_ptr<bool> poolAlive;

        APoolDeleter(APool<T>* pool) : pool(pool), poolAlive(pool->mPoolAlive) {}

        void operator()(T* t) {
            if (*poolAlive) {
                pool->mQueue.push(t);
            } else {
                delete t;
            }
        }
    };

    ~APool() {
        *mPoolAlive = false;
        while (!mQueue.empty()) {
            delete mQueue.front();
            mQueue.pop();
        }
    }
    auto get() noexcept {
        return getImpl<_<T>>();
    }

    using UniquePtr = std::unique_ptr<T, APoolDeleter>;

    auto getUnique() noexcept {
        return getImpl<UniquePtr>();
    }
};

