#pragma once

#include "AUI/Views.h"
#include "AUI/GL/Vao.h"
#include "AUI/GL/Shader.h"
#include "AUI/GL/Texture.h"
#include "AFont.h"
#include "FontStyle.h"
#include "AUI/Common/AColor.h"
#include "AUI/Common/ASide.h"
#include "AUI/Util/Singleton.h"

class AColor;
class AWindow;

class API_AUI_VIEWS Render: public Singleton<Render>
{
public:
	enum Filling
	{
		FILL_SOLID,
        FILL_ROUNDED_SOLID,

		/**
		 * \brief как FILL_SOLID, только с трансформацией
		 */
		FILL_SOLID_TRANSFORM,
		FILL_TEXTURED,
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
	GL::Shader mRoundedSolidShader;
	GL::Shader mSolidTransformShader;
	GL::Shader mTexturedShader;
	GL::Shader mSymbolShader;
	GL::Shader mSymbolShaderSubPixel;
	GL::Vao mTempVao;
	glm::mat4 mTransform;
    Filling mCurrentFill;
	
	glm::mat4 getProjectionMatrix() const;
	AVector<glm::vec3> getVerticesForRect(float x, float y, float width, float height);

	
public:	
	Render();

	void drawRect(float x, float y, float width, float height);
	void drawRoundedRect(float x, float y, float width, float height, float radius);
	void drawLines(const AVector<glm::vec3>& lines);
	void drawRectBorderSide(float x, float y, float width, float height, float lineWidth, ASide s);
	void drawRectBorder(float x, float y, float width, float height, float lineWidth = 1.f);
	
	void setFill(Filling t);
	
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

	void uploadToShader();
};

