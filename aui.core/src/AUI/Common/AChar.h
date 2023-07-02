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


/**
 * @brief Represents a single 32-bit char.
 * @ingroup core
 */
class AChar {
private:
    char32_t mValue;

public:
    AChar(char c): mValue(c) {}

    [[nodiscard]]
    bool digit() const {
        return mValue >= '0' && mValue <= '9';
    }

    [[nodiscard]]
    bool alpha() const {
        return (mValue >= 'a' && mValue <= 'z') || (mValue >= 'A' && mValue <= 'Z');
    }

    [[nodiscard]]
    bool alnum() const {
        return alpha() || digit();
    }

    [[nodiscard]]
    char asAscii() const {
        return char(mValue);
    }

    operator char32_t() const {
        return mValue;
    }
};

static_assert(sizeof(AChar) == 4, "AChar should be exact 4 bytes");


