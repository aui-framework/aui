// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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


#include <array>
#include "AString.h"
#include "AException.h"
#include <AUI/Traits/serializable.h>
#include <AUI/Traits/memory.h>


/**
 * @brief Implements universally unique identifier (UUID)
 * @ingroup core
 */
class API_AUI_CORE AUuid {
private:
    std::array<uint8_t, 16> mData;

    static uint8_t fromHex(char c);

public:
    AUuid() {
        aui::zero(mData);
    }
    explicit AUuid(const std::array<uint8_t, 16>& data) : mData(data) {}


    [[nodiscard]]
    const std::array<uint8_t, 16>& data() const noexcept {
        return mData;
    }

    [[nodiscard]]
    std::array<uint8_t, 16>& data() noexcept {
        return mData;
    }

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
     * Converts a string to UUID. If the string is a valid UUID it is converted to the matching AUuid value else a new
     * UUID generated from the string contents.
     * @param string
     * @return generated UUID.
     */
    static AUuid fromString(const AString& string);

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
        const auto* self = reinterpret_cast<const unsigned*>(mData.data());
        const auto* other = reinterpret_cast<const unsigned*>(o.mData.data());

        for (unsigned i = 0; i < sizeof(mData) / sizeof(unsigned); ++i) {
            if (self[i] != other[i]) {
                return self[i] < other[i];
            }
        };
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

template<>
struct ASerializable<AUuid>: aui::raw_serializable<AUuid> {};