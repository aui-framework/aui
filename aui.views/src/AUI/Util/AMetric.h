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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Core.h>
#include <type_traits>
#include <ostream>
#include <tuple>

class AString;

/**
 * \brief Used to store dimensions in scalable units (dp, pt, etc...).
 * \note It's highly recommended to use only Density-independent Pixel unit (_dp) to make your application correctly
 *       handle systems with high DPI.
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
    AMetric():
        AMetric(0, T_PX) {}

    /**
     * \brief Constructor for AMetric a; a = 0 without unit specifier. Can be used only for zero initialization (see
     *        example)
     * \example
     * <code>
     * AMetric a = 0; // ok<br />
     * AMetric b = 5_dp; // ok<br />
     * AMetric c = 5; // produces error<br />
     * </code>
     * \tparam T integer
     * \param value should be zero
     */
    template<typename T, typename std::enable_if_t<std::is_integral_v<T>, bool> = 0>
    AMetric(T value):
        AMetric(value, T_PX)
    {
        assert(("please use _px, _dp or _pt literal for AMetric initialisation. only zero allowed to initialise "
                "AMetric without literal", value == 0));
    }

    AMetric(float value, Unit unit);
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

    bool operator==(const AMetric& rhs) const {
        return std::tie(mValue, mUnit) == std::tie(rhs.mValue, rhs.mUnit);
    }

    bool operator!=(const AMetric& rhs) const {
        return !(rhs == *this);
    }
};


inline AMetric operator"" _px(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_PX);
}
inline AMetric operator"" _dp(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_DP);
}
inline AMetric operator"" _pt(unsigned long long v)
{
    return AMetric(static_cast<float>(static_cast<long long>(v)), AMetric::T_PT);
}

std::ostream& operator<<(std::ostream& o, const AMetric& value) {
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