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


#include <cassert>

/**
 * @brief Takes care of AddRef and Release() reference counting mechanism of COM objects.
 * @tparam T
 */
template<typename T>
class AComPtr {
public:

    AComPtr() {
        mValue = nullptr;
    }

    ~AComPtr() {
        if (mValue) {
            mValue->Release();
        }
    }

    T** operator&() noexcept {
        assert(("value already set", mValue == nullptr));
        return &mValue;
    }

    AComPtr(T* value): mValue(value) {}
    AComPtr(const AComPtr<T>& rhs): mValue(rhs.mValue) {
        if (mValue) {
            mValue->AddRef();
        }
    }

    AComPtr(AComPtr<T>&& rhs): mValue(rhs.mValue) {
        rhs.mValue = nullptr;
    }

    [[nodiscard]]
    T* value() const noexcept {
        assert(mValue != nullptr);
        return mValue;
    }

    [[nodiscard]]
    T* operator*() const noexcept {
        return value();
    }

    [[nodiscard]]
    T* operator->() const noexcept {
        return value();
    }


    [[nodiscard]]
    operator T*() const noexcept {
        return value();
    }

    [[nodiscard]]
    operator bool() const noexcept {
        return mValue != nullptr;
    }

private:
    T* mValue;
};


