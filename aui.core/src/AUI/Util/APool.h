// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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

