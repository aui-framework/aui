/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <glm/glm.hpp>
#include <AUI/Core.h>
#include <glm/gtx/norm.hpp>
#include <ostream>
#include "AUI/Traits/values.h"

class AString;

/**
 * @brief Represents a 4-component floating point color.
 * @ingroup core
 */
class AColor: public glm::vec4
{
public:
	constexpr AColor(): glm::vec4(0, 0, 0, 1.f)
	{
	}
	constexpr AColor(const glm::vec4& v): glm::vec4(v){}

    API_AUI_CORE AColor(const AString& s);
	constexpr AColor(float scalar) : glm::vec4(scalar) {}
	constexpr AColor(float r, float g, float b) : glm::vec4(r, g, b, 1.f) {}
	constexpr AColor(float r, float g, float b, float a) : glm::vec4(r, g, b, a) {}

	/**
	 * @brief Construct with hex integer
	 * @param color integer representing color in 0xRRGGBBAA
	 * \example AColor(0xff0000ff) will represent opaque bright red
	 */
	constexpr AColor(unsigned int color) : glm::vec4(
		((color >> 24) & 0xff) / 255.f, 
		((color >> 16) & 0xff) / 255.f, 
		((color >> 8) & 0xff) / 255.f, 
		((color) & 0xff) / 255.f) {}

    /**
     * @brief Construct with hex integer
     * @param color integer representing color in 0xAARRGGBB
     * \example AColor(0xff0000ff) will represent opaque bright blue
     */
	static constexpr AColor fromAARRGGBB(unsigned int color)
	{
		return {
		((color >> 16) & 0xff) / 255.f,
		((color >> 8) & 0xff) / 255.f,
		((color) & 0xff) / 255.f,
		((color >> 24) & 0xff) / 255.f,
		};
	}

    /**
     * @brief Construct with hex integer
     * @param color integer representing color in 0xRRGGBB
     * \example AColor(0x00ff00) will represent opaque bright green
     */
	static constexpr AColor fromRRGGBB(unsigned int color)
	{
		return {
		((color >> 16) & 0xff) / 255.f,
		((color >> 8) & 0xff) / 255.f,
		((color) & 0xff) / 255.f,
		1,
		};
	}
	
	constexpr AColor operator*(float other) const
	{
		return AColor(x * other, y * other, z * other, w * other);
	}

    API_AUI_CORE AString toString() const;

    API_AUI_CORE float readabilityOfForegroundColor(const AColor &foreground);


    /**
     * @brief Increases the alpha channel by the given value.
     */
    [[nodiscard]]
    AColor opacify(float alpha) const noexcept {
        AColor c = *this;
        c.a += alpha;
        return c;
    }

    /**
     * @brief Decreases the alpha channel by the given value.
     */
    [[nodiscard]]
    AColor transparentize(float alpha) const noexcept {
        AColor c = *this;
        c.a -= alpha;
        return c;
    }

    /**
     * @brief Multiply all color components except alpha channel (rgb * d, a)
     * @param multiplier
     * @return supplyValue color
     */
    inline constexpr AColor mul(float d) const {
        return glm::clamp(glm::vec4(r * d, g * d, b * d, a), glm::vec4(0.f), glm::vec4(1.f));
    }
    inline constexpr AColor darker(float d) const {
        return mul(1.f - d);
    }
    inline constexpr AColor lighter(float d) const {
        return mul(1.f + d);
    }

    bool isFullyTransparent() const {
        return a < 0.001f;
    }
    bool isFullyOpaque() const {
        return a > 0.999f;
    }

    AColor readableBlackOrWhite() const {
        return glm::length2(glm::vec3{*this}) > 1.5f ? fromRRGGBB(0) : fromRRGGBB(0xffffff);
    }

    AColor opacify(float d) {
        return {x, y, z, a * d};
    }

    static const AColor BLACK;
    static const AColor WHITE;
    static const AColor RED;
    static const AColor GREEN;
    static const AColor BLUE;
    static const AColor GRAY;
};

inline const AColor AColor::BLACK = {0.f, 0.f, 0.f, 1.f};
inline const AColor AColor::WHITE = {1.f, 1.f, 1.f, 1.f};
inline const AColor AColor::RED   = {1.f, 0.f, 0.f, 1.f};
inline const AColor AColor::GREEN = {0.f, 1.f, 0.f, 1.f};
inline const AColor AColor::BLUE  = {0.f, 0.f, 1.f, 1.f};
inline const AColor AColor::GRAY  = {0.5f, 0.5f, 0.5f, 1.f};


inline std::ostream& operator<<(std::ostream& o, const AColor& color) {
    o << "#";
    char buf[16];
    if (!color.isFullyOpaque()) {
        std::snprintf(buf, sizeof(buf), "%02x", uint8_t(color.a * 255.f));
        o << buf;
    }
    std::snprintf(buf, sizeof(buf), "%02x%02x%02x", uint8_t(color.r * 255.f), uint8_t(color.g * 255.f), uint8_t(color.b * 255.f));
    o << buf;
    return o;
}

/**
 * @brief Construct with hex integer
 * @param color integer representing color in 0xAARRGGBB
 * \example AColor(0xff0000ff) will represent opaque bright blue
 */
inline constexpr AColor operator"" _argb(unsigned long long v)
{
    return AColor::fromAARRGGBB(unsigned(v));
}


/**
 * @brief Construct with hex integer
 * @param color integer representing color in 0xRRGGBB
 * \example AColor(0x00ff00) will represent opaque bright green
 */
inline constexpr AColor operator"" _rgb(unsigned long long v)
{
	assert(("_rgb literal should be in 0xrrggbb format, not 0xaarrggbb" && !(v & 0xff000000u)));
    return AColor::fromRRGGBB(unsigned(v));
}


struct API_AUI_CORE AColorHSV {
    aui::float_within_0_1 hue = 0.f;
    aui::float_within_0_1 saturation = 0.f;
    aui::float_within_0_1 value = 0.f;

    [[nodiscard]] static AColorHSV fromRGB(AColor color) noexcept;

    [[nodiscard]] AColor toRGB() const noexcept;
};