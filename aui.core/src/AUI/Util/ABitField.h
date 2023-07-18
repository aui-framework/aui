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
 * @brief Bit field implementation.
 */
template <typename T = uint32_t>
class ABitField {
private:
    T mStorage;

public:
    ABitField(T storage = static_cast<T>(0)) : mStorage(storage) {}

    operator T() {
        return mStorage;
    }

    T& value() {
        return mStorage;
    }

    const T& value() const {
        return mStorage;
    }

    /**
     * @brief Sets flag.
     * @param flag flag
     * @return this
     */
    ABitField& operator<<(T flag) {
        mStorage |= flag;
        return *this;
    }

    /**
     * @brief Resets flag.
     * @param flag flag
     * @return this
     */
    ABitField& operator>>(T flag) {
        mStorage &= ~flag;
        return *this;
    }

    /**
     * @brief Determines whether flag set or not and resets flag.
     * @param flag flag
     * @return true if flag was set
     */
    bool checkAndSet(T flag) {
        if (!!(mStorage & flag)) {
            mStorage &= ~flag;
            return true;
        }
        return false;
    }

    /**
     * @brief Determines whether flag set or not and sets flag.
     * @param flag flag
     * @return true if flag was reset
     */
    bool checkAndReset(T flag)
    {
        if (mStorage & flag)
        {
            return false;
        }
        mStorage |= flag;
        return true;
    }

    /**
     * @brief Determines whether flag (or all flags) set or not.
     * @param flags flag(s)
     * @return true if flag(s) set
     * @note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool test(T flags) const {
        return (mStorage & flags) == flags;
    }
    /**
     * @brief Determines whether flag (or one of the flags flags) set or not.
     * @param flags flag(s)
     * @return true if flag(s) set
     * @note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool testAny(T flags) const {
        return bool(mStorage & flags);
    }

    /**
     * @brief Determines whether flag(s) set or not.
     * @param flags flag(s)
     * @return true if flag(s) set
     * @note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool operator&(T flags) const {
        return test(flags);
    }

    void set(T flag, bool value) {
        if (value) {
            mStorage |= flag;
        } else {
            mStorage &= ~flag;
        }
    }
};