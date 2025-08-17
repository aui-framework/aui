/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
 * ## Initialization
 * Recommended way is to use operator literal format:
 * ```cpp
 * auto angle = 180_deg;
 * angle.radians() --> 3.14
 * angle.degrees() --> 180
 * ```
 * ```cpp
 * auto angle = 3.14_rad;
 * angle.radians() --> 3.14
 * angle.degrees() --> 180
 * ```
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

    AAngleRadians& operator+=(AAngleRadians rhs) noexcept {
        mValue += rhs.mValue;
        return *this;
    }

    AAngleRadians& operator-=(AAngleRadians rhs) noexcept {
        mValue -= rhs.mValue;
        return *this;
    }

    AAngleRadians& operator*=(float k) noexcept {
        mValue *= k;
        return *this;
    }

    AAngleRadians& operator/=(float k) noexcept {
        mValue /= k;
        return *this;
    }

    AAngleRadians operator+(AAngleRadians rhs) const noexcept {
        return AAngleRadians(mValue + rhs.mValue);
    }

    AAngleRadians operator-(AAngleRadians rhs) const noexcept {
        return AAngleRadians(mValue - rhs.mValue);
    }

    AAngleRadians operator*(float k) const noexcept {
        return AAngleRadians(mValue * k);
    }

    AAngleRadians operator/(float k) const noexcept {
        return AAngleRadians(mValue / k);
    }

private:
    float mValue = 0.f;
};


constexpr inline AAngleRadians operator""_rad(long double v)
{
    return AAngleRadians(v);
}
constexpr inline AAngleRadians operator""_deg(long double v)
{
    return AAngleRadians(glm::radians(v));
}
constexpr inline AAngleRadians operator""_deg(unsigned long long v)
{
    return AAngleRadians(glm::radians(float(v)));
}

inline std::ostream& operator<<(std::ostream& o, const AAngleRadians& value) {
    o << value.degrees() << "deg";
    return o;
}