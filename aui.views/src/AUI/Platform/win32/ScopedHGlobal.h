// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <Windows.h>

/**
 * @brief RAII helper to access HGLOBAL data.
 */
class ScopedHGlobal {
public:
    explicit ScopedHGlobal(HGLOBAL blob) noexcept: mBlob(blob) {
        mAccessibleData = static_cast<char*>(GlobalLock(blob));
    }

    ~ScopedHGlobal() {
        GlobalUnlock(mBlob);
    }

    [[nodiscard]]
    char* data() const noexcept {
        return mAccessibleData;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return GlobalSize(mBlob);
    }

    operator AByteBufferView() const noexcept {
        return { data(), size() };
    }

private:
    HGLOBAL mBlob;
    char* mAccessibleData;
};