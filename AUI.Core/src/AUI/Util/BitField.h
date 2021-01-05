/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once


/**
 * \brief Bit field implementation.
 */
template <typename T = uint32_t>
class BitField {
private:
    T mStorage;

public:
    BitField(T storage = static_cast<T>(0)) : mStorage(storage) {}

    operator T() {
        return mStorage;
    }

    T& storage() {
        return mStorage;
    }

    /**
     * \brief Sets flag.
     * \param flag flag
     * \return this
     */
    BitField& operator<<(T flag) {
        mStorage |= flag;
        return *this;
    }

    /**
     * \brief Resets flag.
     * \param flag flag
     * \return this
     */
    BitField& operator>>(T flag) {
        mStorage &= ~flag;
        return *this;
    }

    /**
     * \brief Determines whether flag set or not and resets flag.
     * \param flag flag
     * \return true if flag was set
     */
    bool checkAndSet(T flag) {
        if (mStorage & flag) {
            mStorage &= ~flag;
            return true;
        }
        return false;
    }

    /**
     * \brief Determines whether flag set or not and sets flag.
     * \param flag flag
     * \return true if flag was reset
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
     * \brief Determines whether flag(s) set or not.
     * \param flags flag(s)
     * \return true if flag(s) set
     * \note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool check(T flags) const {
        return (mStorage & flags) == flags;
    }

    /**
     * \brief Determines whether flag(s) set or not.
     * \param flags flag(s)
     * \return true if flag(s) set
     * \note This function supports multiple flags (i.e <code>check(FLAG1 | FLAG2)</code>).
     */
    bool operator&(T flags) const {
        return check(flags);
    }
};