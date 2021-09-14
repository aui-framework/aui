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

#include <glm/glm.hpp>
#include <AUI/Core.h>
#include <glm/gtx/norm.hpp>

class AString;

class API_AUI_CORE AColor: public glm::vec4
{
public:
	AColor(): glm::vec4(0, 0, 0, 1.f)
	{
	}
	AColor(const glm::vec4& v): glm::vec4(v){}
	
	AColor(const AString& s);
	AColor(float scalar) : glm::vec4(scalar) {}
	AColor(float r, float g, float b) : glm::vec4(r, g, b, 1.f) {}
	AColor(float r, float g, float b, float a) : glm::vec4(r, g, b, a) {}

	/**
	 * \brief Construct with hex integer
	 * \param color integer representing color in 0xRRGGBBAA
	 * \example AColor(0xff0000ff) will represent opaque bright red
	 */
	AColor(unsigned int color) : glm::vec4(
		((color >> 24) & 0xff) / 255.f, 
		((color >> 16) & 0xff) / 255.f, 
		((color >> 8) & 0xff) / 255.f, 
		((color) & 0xff) / 255.f) {}

    /**
     * \brief Construct with hex integer
     * \param color integer representing color in 0xAARRGGBB
     * \example AColor(0xff0000ff) will represent opaque bright blue
     */
	static AColor fromAARRGGBB(unsigned int color)
	{
		return {
		((color >> 16) & 0xff) / 255.f,
		((color >> 8) & 0xff) / 255.f,
		((color) & 0xff) / 255.f,
		((color >> 24) & 0xff) / 255.f,
		};
	}

    /**
     * \brief Construct with hex integer
     * \param color integer representing color in 0xRRGGBB
     * \example AColor(0x00ff00) will represent opaque bright green
     */
	static AColor fromRRGGBB(unsigned int color)
	{
		return {
		((color >> 16) & 0xff) / 255.f,
		((color >> 8) & 0xff) / 255.f,
		((color) & 0xff) / 255.f,
		1,
		};
	}
	
	AColor operator*(float other) const
	{
		return AColor(x * other, y * other, z * other, w);
	}

	AString toString() const;

    float readabilityOfForegroundColor(const AColor &foreground);

    /**
     * \brief Multiply all color components except alpha channel (rgb * d, a)
     * \param multiplier
     * \return result color
     */
    inline AColor mul(float d) {
        return AColor(r * d, g * d, b * d, a);
    }
    inline AColor darker(float d) {
        return mul(1.f - d);
    }
    inline AColor lighter(float d) {
        return mul(1.f + d);
    }

    bool isFullyTransparent() const {
        return a < 0.001f;
    }

    AColor readableBlackOrWhite() const {
        return glm::length2(glm::vec3{*this}) > 1.5f ? fromRRGGBB(0) : fromRRGGBB(0xffffff);
    }

    AColor opacify(float d) {
        return {x, y, z, a * d};
    }
};

/**
 * \brief Construct with hex integer
 * \param color integer representing color in 0xAARRGGBB
 * \example AColor(0xff0000ff) will represent opaque bright blue
 */
inline AColor operator"" _argb(unsigned long long v)
{
    return AColor::fromAARRGGBB(unsigned(v));
}


/**
 * \brief Construct with hex integer
 * \param color integer representing color in 0xRRGGBB
 * \example AColor(0x00ff00) will represent opaque bright green
 */
inline AColor operator"" _rgb(unsigned long long v)
{
	assert(("_rgb literal should be in 0xrrggbb format, not 0xaarrggbb", !(v & 0xff000000u)));
    return AColor::fromRRGGBB(unsigned(v));
}

