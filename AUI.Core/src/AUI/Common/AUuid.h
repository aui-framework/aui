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


#include <array>
#include "AString.h"
#include "AException.h"


/**
 * AUI universally unique identifier (UUID) implementation
 */
class AUuid {
private:
    std::array<uint8_t, 16> mData;

    static uint8_t fromHex(char c);

public:
    explicit AUuid(const std::array<uint8_t, 16>& data) : mData(data) {}

    /**
     * Construct UUID value from string. Supported the following formats:
     * <ul>
     *     <li>Canonical: <code>8-4-4-4-12</code> (<code>123e4567-e89b-12d3-a456-426655440000</code>)</li>
     *     <li>Raw: <code>32</code> (<code>123e4567e89b12d3a456426655440000</code>)</li>
     * </ul>
     * @param s string representing UUID
     */
    AUuid(const AString& s);

    /**
     * @return uuid string in canonical format
     */
    [[nodiscard]] AString toString() const;

    /**
     * @return uuid string in raw format
     */
    [[nodiscard]] AString toRawString() const;

    operator AString() const {
        return toString();
    }


    bool operator<(const AUuid& o) const {
        const auto* self = reinterpret_cast<const uint32_t*>(mData.data());
        const auto* other = reinterpret_cast<const uint32_t*>(o.mData.data());

        for (size_t i = 0; i < 4; ++i) {
            if (self[i] < other[i]) {
                return true;
            }
        }
        return false;
    }

    bool operator==(const AUuid& other) const {
        return mData == other.mData;
    }

    bool operator!=(const AUuid& other) const {
        return mData != other.mData;
    }
};

inline std::ostream& operator<<(std::ostream& o, const AUuid& u) {
    o << "urn:uuid:" << u.toString();
    return o;
}


class AUuidException: public AException {
private:
    friend class AUuid;

    using AException::AException;
public:
};