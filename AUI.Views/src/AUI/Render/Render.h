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

#include "AUI/GL/Vao.h"
#include "AUI/GL/Shader.h"
#include "AUI/GL/Texture2D.h"
#include "AFont.h"
#include "FontStyle.h"
#include "AUI/Common/AColor.h"
#include "AUI/Common/ASide.h"


class AColor;
class AWindow;



ENUM_FLAG(Repeat) {
    NONE = 0,
    X_Y = 0b11,
    X = 0b01,
    Y = 0b10,
};

class API_AUI_VIEWS Render
{
public:
	enum Filling
	{
		FILL_SOLID,

		/**
		 * \brief FILL_SOLID but with transform matrix
		 */
		FILL_SOLID_TRANSFORM,
		FILL_TEXTURED,
		FILL_GRADIENT,
		FILL_SYMBOL,
		FILL_SYMBOL_SUBPIXEL,
	};

	class PrerenderedString {
	public:
		PrerenderedString():
			length(0),
			side(0)
		{};

		PrerenderedString(const _<GL::Vao>& mVao, const FontStyle& fs, float length,
                          uint16_t side, const AString& mText) : mVao(mVao), fs(fs), length(length),
			side(side), mText(mText) {}

		_<GL::Vao> mVao;
		FontStyle fs;
		float length;
		uint16_t side;
		AString mText;

        inline void invalidate() {
            mVao = nullptr;
        }
    };
private:
	AWindow* mWindow = nullptr;
	AColor mColor = 1.f;

	AColor mGradientTL = 1.f;
	AColor mGradientTR = 1.f;
	AColor mGradientBL = 1.f;
	AColor mGradientBR = 1.f;

	GL::Shader mSolidShader;
	GL::Shader mGradientShader;
	GL::Shader mRoundedSolidShader;
	GL::Shader mRoundedSolidShaderAntialiased;
	GL::Shader mRoundedGradientShaderAntialiased;
	GL::Shader mSolidTransformShader;
	GL::Shader mBoxShadowShader;
	GL::Shader mTexturedShader;
	GL::Shader mSymbolShader;
	GL::Shader mSymbolShaderSubPixel;
	GL::Vao mTempVao;
	glm::mat4 mTransform;
    Filling mCurrentFill;

    /**
     * \brief Repeating. Handled by IDrawable.
     */
    Repeat mRepeat = Repeat::NONE;
	
	glm::mat4 getProjectionMatrix() const;
	AVector<glm::vec3> getVerticesForRect(float x, float y, float width, float height);

    void uploadToShaderCommon();
    void uploadToShaderGradient();

	
public:	
	Render();

	static Render& inst();

	void drawRect(float x, float y, float width, float height);
	void drawTexturedRect(float x, float y, float width, float height,
                       const glm::vec2& uv1 = {0, 0}, const glm::vec2& uv2 = {1, 1});
    void drawRoundedRect(float x, float y, float width, float height, float radius);
    void drawRoundedRectAntialiased(float x, float y, float width, float height, float radius);
    void drawRectBorderSide(float x, float y, float width, float height, float lineWidth, ASide s);
	void drawRectBorder(float x, float y, float width, float height, float lineWidth = 1.f);
    void drawRoundedBorder(float x, float y, float width, float height, float radius, int borderWidth);

	void setFill(Filling t);

	void setGradientColors(const AColor& tl, const AColor& tr,
						   const AColor& bl, const AColor& br);

	void setWindow(AWindow* const window)
	{
		mWindow = window;
		setColorForced(1.f);
		setTransformForced(getProjectionMatrix());
	}
    inline void drawString(int x, int y, const AString& text)
	{
		FontStyle fs;
		drawString(x, y, text, fs);
	}
    void drawBoxShadow(float x, float y, float width, float height, float blurRadius, const AColor& color);
    void drawString(int x, int y, const AString& text, FontStyle& fs);
    void drawString(int x, int y, PrerenderedString& f);

	PrerenderedString preRendererString(const AString& text, FontStyle& fs);

	void setColorForced(const AColor& color)
	{
		mColor = color;
	}

	void setColor(const AColor& color)
	{
		setColorForced(mColor * color);
	}

	const AColor& getColor() const
	{
		return mColor;
	}

	void setTransform(const glm::mat4& transform)
	{
		mTransform *= transform;
	}

	void setTransformForced(const glm::mat4& transform)
	{
		mTransform = transform;
	}

	const glm::mat4& getTransform()
	{
		return mTransform;
	}
    Repeat getRepeat() const {
	    return mRepeat;
	}

	void setRepeat(Repeat repeat) {
	    mRepeat = repeat;
	}

    glm::vec2 getCurrentPos();

    void applyTextureRepeat();
};

