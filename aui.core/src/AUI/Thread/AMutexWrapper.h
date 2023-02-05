// AUI Framework - Declarative UI toolkit for modern C++20
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

#include <AUI/Traits/values.h>

/**
 * @brief Wraps the object with mutex, providing thread-safety layer and a runtime check.
 * @ingroup core
 */
template<typename T>
class AMutexWrapper: public aui::noncopyable {
public:
    AMutexWrapper(T value = T()) noexcept: mValue(std::move(value)) {}


    void lock() {
        mMutex.lock();

#if AUI_DEBUG
        mOwnerThread = std::this_thread::get_id();
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
        mMutex.unlock();
#if AUI_DEBUG
        mOwnerThread = std::this_thread::get_id();
#endif
    }

    T& value() noexcept {
#if AUI_DEBUG
        assert(("AMutexWrapper should be locked by this thread in order to get access to the underlying object", mOwnerThread == std::this_thread::get_id()));
#endif
        return mValue;
    }

    T* operator->() noexcept {
        return &value();
    }

private:
    T mValue;
    AMutex mMutex;

#if AUI_DEBUG
    std::thread::id mOwnerThread;
#endif
};