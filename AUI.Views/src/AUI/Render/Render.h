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
    REPEAT_NONE = 0,
    REPEAT = 0b11,
    REPEAT_X = 0b01,
    REPEAT_Y = 0b10,
};

class API_AUI_VIEWS Render
{
public:
	enum Filling
	{
		FILL_SOLID,

		/**
		 * \brief как FILL_SOLID, только с трансформацией
		 */
		FILL_SOLID_TRANSFORM,
		FILL_TEXTURED,
		FILL_GRADIENT,
		FILL_SYMBOL,
		FILL_SYMBOL_SUBPIXEL,
	};

	class PrerendereredString {
	public:
		PrerendereredString():
			length(0),
			side(0)
		{};

		PrerendereredString(const _<GL::Vao>& mVao, const FontStyle& fs, float length,
			uint16_t side, const AString& mText) : mVao(mVao), fs(fs), length(length),
			side(side), mText(mText) {}

		_<GL::Vao> mVao;
		FontStyle fs;
		float length;
		uint16_t side;
		AString mText;
	};
private:
	AWindow* mWindow = nullptr;
	AColor mColor = 1.f;
	GL::Shader mSolidShader;
	GL::Shader mGradientShader;
	GL::Shader mRoundedSolidShader;
	GL::Shader mRoundedSolidShaderAntialiased;
	GL::Shader mSolidTransformShader;
	GL::Shader mBoxShadowShader;
	GL::Shader mTexturedShader;
	GL::Shader mSymbolShader;
	GL::Shader mSymbolShaderSubPixel;
	GL::Vao mTempVao;
	glm::mat4 mTransform;
    Filling mCurrentFill;

    /**
     * \brief Повторение. Обрабатываются IDrawabl'ами самостоятельно.
     */
    Repeat mRepeat = REPEAT_NONE;
	
	glm::mat4 getProjectionMatrix() const;
	AVector<glm::vec3> getVerticesForRect(float x, float y, float width, float height);

	
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
    void drawString(int x, int y, PrerendereredString& f);

	PrerendereredString preRendererString(const AString& text, FontStyle& fs);

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

	void uploadToShader();

    glm::vec2 getCurrentPos();

    void applyTextureRepeat();
};

