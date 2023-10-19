//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Core.h>
#include <type_traits>
#include <cassert>
#include <ostream>
#include <tuple>
#include <glm/glm.hpp>

class AString;

/**
 * @brief Strong type used to store angle in radians.
 * @ingroup views
 * @details
 *
 * # Initialization
 * Recommended way is to use operator literal format:
 * @code{cpp}
 * auto angle = 180_deg;
 * angle.radians() --> 3.14
 * angle.degrees() --> 180
 * @endcode
 * @code{cpp}
 * auto angle = 3.14_rad;
 * angle.radians() --> 3.14
 * angle.degrees() --> 180
 * @endcode
 */
class API_AUI_VIEWS AAngleRadians
{
public:
    constexpr AAngleRadians() noexcept = default;
    constexpr explicit AAngleRadians(float radians) noexcept: mValue(radians) {}

    [[nodiscard]]
    constexpr float radians() const noexcept {
        return mValue;
    }

    [[nodiscard]]
    constexpr float degrees() const noexcept {
        return glm::degrees(mValue);
    }

private:
    float mValue = 0.f;
};


constexpr inline AAngleRadians operator"" _rad(long double v)
{
    return AAngleRadians(v);
}
constexpr inline AAngleRadians operator"" _deg(long double v)
{
    return AAngleRadians(glm::radians(v));
}
constexpr inline AAngleRadians operator"" _deg(unsigned long long v)
{
    return AAngleRadians(glm::radians(float(v)));
}

inline std::ostream& operator<<(std::ostream& o, const AAngleRadians& value) {
    o << value.degrees() << "deg";
    return o;
}