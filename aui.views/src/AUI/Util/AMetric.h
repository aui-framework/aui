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
#include <ostream>
#include <tuple>
#include "AUI/Util/Assert.h"

class AString;

/**
 * @brief Stores dimensions in scalable units (dp, pt, etc...).
 * @ingroup views
 * @details
 * It's highly recommended to use only Density-independent Pixel unit (`_dp`) to make your application correctly handle
 * systems with high DPI. In AUI, all units are density independent except `_px`. The only purpose of the `_px` unit is to
 * define lines of exact one or two pixels wide.
 *
 * ## Initialization
 * Recommended way is to use operator literal format:
 * ```cpp
 * AMetric a = 5_dp // -> a = 5 dimension-independent units (= pixels on 100% scaling)
 * ```
 *
 * Common usage:
 * ```cpp
 * AMetric a = 5_dp;
 * a.getValuePx() // 5 on 100% scale, 6 on 125% scale, etc
 * ```
 *
 * AMetric can be also initialized via value and unit:
 *
 * ```cpp
 * AMetric a(5, T_DP);
 * ```
 *
 * AMetric can be also initialized with zero without unit specified (in this case, AMetric::getUnit will return T_PX):
 *
 * ```cpp
 * AMetric zero1 = 0; // zero pixels
 * AMetric zero2 = {}; // also zero pixels
 * ```
 *
 * However, if you try to specify nonzero integer without unit, it will produce a runtime error:
 *
 * ```cpp
 * AMetric a = 5; // runtime error
 * ```
 *
 * ## Supported units
 *
 * <!-- aui:index_alias _dp -->
 * <!-- aui:index_alias _pt -->
 * <!-- aui:index_alias _px -->
 *
 * Currently supported units:
 *
 * | Unit                       | Enum | Literal | Value                       |
 * | -------------------------- | ---- | ------- | --------------------------- |
 * | Density-independent Pixels | T_DP | _dp     | px * `scale_factor`         |
 * | Typography point           | T_PT | _pt     | px * `scale_factor` * 4 / 3 |
 * | Pixels                     | T_PX | _px     | px                          |
 *
 * It's highly recommended to use only Density-independent Pixel unit (_dp). DP guarantees that your application
 * will correctly handle systems with hidpi screens.
 */
class API_AUI_VIEWS AMetric
{
public:
    enum Unit
    {
        T_UNKNOWN,
        T_PX,
        T_DP,
        T_PT,
    };

private:
    float mValue;
    Unit mUnit;

    float convertValueToMyUnit(const AMetric& anotherMetric) {
        return fromPxToMetric(anotherMetric.getValuePx(), mUnit);
    }

public:
    constexpr AMetric():
        AMetric(0, T_PX) {}

    /**
     * @brief Constructor for AMetric a; a = 0 without unit specifier. Can be used only for zero initialization (see
     *        example)
     * ```cpp
     * AMetric a = 0; // ok<br />
     * AMetric b = 5_dp; // ok<br />
     * AMetric c = 5; // produces error<br />
     * ```
     * @tparam T integer
     * @param value should be zero
     */
    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = 0>
    constexpr AMetric(T value):
        AMetric(value, T_PX)
    {
        AUI_ASSERTX(value == 0,
                    "please use _px, _dp or _pt literal for AMetric initialisation. only zero allowed to "
                    "initialise AMetric without literal");
    }

    constexpr AMetric(float value, Unit unit): mValue(value), mUnit(unit) {

    }

    AMetric(const AString& text);

    [[nodiscard]] float getRawValue() const
    {
        return mValue;
    }

    [[nodiscard]] Unit getUnit() const
    {
        return mUnit;
    }

    [[nodiscard]] float getValuePx() const;
    [[nodiscard]] float getValueDp() const;

    static float fromPxToMetric(float value, Unit unit);

    operator float() const {
        return getValuePx();
    }
    
    AMetric operator-() const {
        return {-mValue, mUnit};
    }

    AMetric& operator+=(AMetric rhs) noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        mValue += rhs.mValue;
        return *this;
    }

    AMetric& operator-=(AMetric rhs) noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        mValue -= rhs.mValue;
        return *this;
    }

    AMetric& operator*=(AMetric rhs) noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        mValue *= rhs.mValue;
        return *this;
    }

    AMetric& operator/=(AMetric rhs) noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        mValue /= rhs.mValue;
        return *this;
    }

    AMetric operator+(AMetric rhs) const noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        auto copy = *this;
        copy += rhs;
        return copy;
    }

    AMetric operator-(AMetric rhs) const noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        auto copy = *this;
        copy -= rhs;
        return copy;
    }

    AMetric operator*(AMetric rhs) const noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        auto copy = *this;
        copy *= rhs;
        return copy;
    }

    AMetric operator/(AMetric rhs) const noexcept {
        AUI_ASSERT(mUnit == rhs.mUnit);
        auto copy = *this;
        copy /= rhs;
        return copy;
    }

    AMetric& operator*=(float rhs) noexcept {
        mValue *= rhs;
        return *this;
    }

    AMetric& operator/=(float rhs) noexcept {
        mValue /= rhs;
        return *this;
    }

    AMetric operator*(float rhs) const noexcept {
        auto copy = *this;
        copy *= rhs;
        return copy;
    }

    AMetric operator/(float rhs) const noexcept {
        auto copy = *this;
        copy /= rhs;
        return copy;
    }

    bool operator==(const AMetric& rhs) const {
        return std::tie(mValue, mUnit) == std::tie(rhs.mValue, rhs.mUnit);
    }

    bool operator!=(const AMetric& rhs) const {
        return !(rhs == *this);
    }
};


constexpr inline AMetric operator""_px(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_PX);
}
constexpr inline AMetric operator""_dp(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_DP);
}
constexpr inline AMetric operator""_pt(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_PT);
}

inline std::ostream& operator<<(std::ostream& o, const AMetric& value) {
    o << value.getRawValue();
    switch (value.getUnit()) {
        case AMetric::T_PX:
            o << "_px";
            break;
        case AMetric::T_DP:
            o << "_dp";
            break;
        case AMetric::T_PT:
            o << "_pt";
            break;

        default:
            break;
    }
    return o;
}