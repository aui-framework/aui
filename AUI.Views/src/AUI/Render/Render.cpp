#include "Render.h"

#include <glm/gtc/matrix_transform.hpp>
#include "AFontManager.h"
#include <AUI/Common/AColor.h>
#include <AUI/Common/ASide.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Render/AFontCharacter.h>

Render::Render()
{
	mSolidShader.load(
		"attribute vec3 pos;"
		"void main(void) {gl_Position = vec4(pos, 1);}",
		"uniform vec4 color;"
		"void main(void) {gl_FragColor = color;}");
    mBoxShadowShader.load(
            "attribute vec3 pos;"
            "uniform mat4 transform;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = transform * vec4(pos, 1); pass_uv = pos.xy;}",
        "varying vec2 pass_uv;"
        "uniform vec4 color;"
        "uniform vec2 lower;"
        "uniform vec2 upper;"
        "uniform float sigma;"
        "vec4 erf(vec4 x) {"
            "vec4 s = sign(x), a = abs(x);"
            "x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;"
            "x *= x;"
            "return s - s / (x * x);"
        "}"
        "void main(void) {"
            "gl_FragColor = color;"

            "vec4 query = vec4(pass_uv - vec2(lower), pass_uv - vec2(upper));"
            "vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));"
            "gl_FragColor.a *= clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0);"
            //"gl_FragColor.a = query.x + query.y;"
         "}");
    /*

    if (glewGetExtension("ARB_multisample")) {
        mRoundedSolidShader.load(
                "attribute vec3 pos;"
                "attribute vec2 uv;"
                "varying vec2 pass_uv;"
                "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                "uniform vec2 size;"
                "varying vec2 pass_uv;"
                "void main(void) {"
                "vec2 tmp = abs(pass_uv);"
                "if ((tmp.x - 1.0) * (size.y) / (-size.x) < tmp.y - (1.0 - size.y) &&"
                "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) > 1.0) discard;"
                "}");
    } else */ {
        // без сглаживания скруглённые края выглядят убого. исправим это
        mRoundedSolidShader.load(
                "attribute vec3 pos;"
                "attribute vec2 uv;"
                "varying vec2 pass_uv;"
                "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                "uniform vec2 size;"
                "varying vec2 pass_uv;"
                "void main(void) {"
                "vec2 tmp = abs(pass_uv);"
                "if ((tmp.x - 1.0) * (size.y) / (-size.x) < tmp.y - (1.0 - size.y) &&"
                "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) > 1.0) discard;"
                "}");
        mRoundedSolidShaderAntialiased.load(
                "attribute vec3 pos;"
                "attribute vec2 uv;"
                "attribute vec2 outer_to_inner;"
                "varying vec2 pass_uv;"
                "void main(void) {gl_Position = vec4(pos, 1.0); pass_uv = uv * 2.0 - vec2(1.0, 1.0);}",
                "uniform vec2 outerSize;"
                "uniform vec2 innerSize;"
                "uniform vec2 innerTexelSize;"
                "uniform vec2 outerTexelSize;"
                "uniform vec2 outer_to_inner;"
                "uniform vec4 color;"
                "varying vec2 pass_uv;"
                "bool is_outside(vec2 tmp, vec2 size) {"
                    "if (tmp.x >= 1 || tmp.y >= 1) return true;"
                    "return (tmp.x - 1.0) * (size.y) / (-size.x) <= tmp.y - (1.0 - size.y) &&"
                        "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                        "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) >= 1.0;"
                "}"
                "void main(void) {"
                    "vec2 outer_uv = abs(pass_uv);"
                    "vec2 inner_uv = outer_uv * outer_to_inner;"
                    "float alpha = 1.f;"
                    "ivec2 i;"
                    "for (i.x = -2; i.x <= 2; ++i.x) {"
                        "for (i.y = -2; i.y <= 2; ++i.y) {"
                        "alpha -= (is_outside(inner_uv + innerTexelSize * (i), innerSize)"
                                  " == "
                                  "is_outside(outer_uv + outerTexelSize * (i), outerSize)"
                                  ") ? (1.0 / 25.0) : 0;"
                        "}"
                    "}"
                    "gl_FragColor = vec4(color.rgb, color.a * alpha);"
                "}");
    }

	mSolidTransformShader.load(
		"attribute vec3 pos;"
		"uniform mat4 transform;"
		"void main(void) {gl_Position = transform * vec4(pos, 1);}",
		"uniform vec4 color;"
		"void main(void) {gl_FragColor = color;}");

	mTexturedShader.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv;}",
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"varying vec2 pass_uv;"
		"void main(void) {gl_FragColor = texture2D(tex, pass_uv) * color; if (gl_FragColor.a < 0.01) discard;}",
		{"pos", "", "uv"});

	mGradientShader.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv;}",
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"uniform vec4 color_tl;"
		"uniform vec4 color_tr;"
		"uniform vec4 color_bl;"
		"uniform vec4 color_br;"
		"varying vec2 pass_uv;"
		"void main(void) {gl_FragColor = mix(mix(color_tl, color_tr, pass_uv.x), mix(color_bl, color_br, pass_uv.x), pass_uv.y) * color;}",
		{"pos", "uv"});

	mSymbolShader.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"uniform mat4 mat;"

		"void main(void) {gl_Position = mat * vec4(pos, 1); pass_uv = uv;}",
		"varying vec2 pass_uv;"
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"void main(void) {float sample = pow(texture2D(tex, pass_uv).r, 1.0 / 1.2); gl_FragColor = vec4(color.rgb, color.a * sample); if (gl_FragColor.a < 0.01) discard;}",
		{"pos", "uv"});

	mSymbolShaderSubPixel.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"uniform mat4 mat;"

		"void main(void) {gl_Position = mat * vec4(pos, 1); pass_uv = uv;}",
		"varying vec2 pass_uv;"
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"void main(void) {vec3 sample = pow(texture2D(tex, pass_uv).rgb, vec3(1.0 / 1.2)); gl_FragColor = vec4(sample * color.rgb * color.a, 1);}",
		{"pos", "uv"});

	mTempVao.bind();

	mTempVao.insert(1, {
						{0, 1},
						{1, 1},
						{0, 0},
						{1, 0}
		}
	);

	setFill(FILL_SOLID);
}


glm::mat4 Render::getProjectionMatrix() const
{
	return glm::ortho(0.f, static_cast<float>(mWindow->getWidth()), static_cast<float>(mWindow->getHeight()), 0.f);
}

AVector<glm::vec3> Render::getVerticesForRect(float x, float y, float width, float height)
{
	float w = x + width;
	float h = y + height;

	return
	{
		glm::vec3(mTransform * glm::vec4{ x, h, 1, 1 }),
		glm::vec3(mTransform * glm::vec4{ w, h, 1, 1 }),
		glm::vec3(mTransform * glm::vec4{ x, y, 1, 1 }),
		glm::vec3(mTransform * glm::vec4{ w, y, 1, 1 }),
	};
}

void Render::drawRect(float x, float y, float width, float height)
{
	uploadToShader();
	mTempVao.bind();

	mTempVao.insert(0, getVerticesForRect(x, y, width, height));

	mTempVao.indices({ 0, 1, 2, 2, 1, 3 });
	mTempVao.draw();
}

void Render::drawTexturedRect(float x, float y, float width, float height, const glm::vec2& uv1, const glm::vec2& uv2) {
    uploadToShader();
    mTempVao.bind();
    glEnableVertexAttribArray(2);

    mTempVao.insert(0, getVerticesForRect(x, y, width, height));
    mTempVao.insert(2, {
        glm::vec2{uv1.x, uv2.y},
        glm::vec2{uv2.x, uv2.y},
        glm::vec2{uv1.x, uv1.y},
        glm::vec2{uv2.x, uv1.y},
    });

    mTempVao.indices({ 0, 1, 2, 2, 1, 3 });
    mTempVao.draw();
    glDisableVertexAttribArray(2);
}

void Render::drawRoundedRect(float x, float y, float width, float height, float radius) {
    mRoundedSolidShader.use();

    mRoundedSolidShader.set("size", 2.f * radius / glm::vec2{width, height});
    drawRect(x, y, width, height);
    setFill(mCurrentFill);
}
void Render::drawRoundedRectAntialiased(float x, float y, float width, float height, float radius) {
    mRoundedSolidShaderAntialiased.use();
    mRoundedSolidShaderAntialiased.set("outerSize", 2.f * radius / glm::vec2{width, height});
    mRoundedSolidShaderAntialiased.set("innerSize", glm::vec2{9999});
    mRoundedSolidShaderAntialiased.set("innerTexelSize", glm::vec2{0, 0});
    mRoundedSolidShaderAntialiased.set("outerTexelSize", 2.f / 5.f / glm::vec2{width, height});
    mRoundedSolidShaderAntialiased.set("outer_to_inner", glm::vec2{0});
    drawRect(x, y, width, height);
    setFill(mCurrentFill);
}
void Render::drawRoundedBorder(float x, float y, float width, float height, float radius, int borderWidth) {
    //float innerX = x + borderWidth;
    //float innerY = y + borderWidth;
    float innerWidth = width - borderWidth * 2;
    float innerHeight = height - borderWidth * 2;

    mRoundedSolidShaderAntialiased.use();
    mRoundedSolidShaderAntialiased.set("outerSize", 2.f * radius / glm::vec2{width, height});
    mRoundedSolidShaderAntialiased.set("innerSize", 2.f * (radius - borderWidth) / glm::vec2{innerWidth, innerHeight});
    mRoundedSolidShaderAntialiased.set("outer_to_inner", glm::vec2{width, height} / glm::vec2{innerWidth, innerHeight});

    mRoundedSolidShaderAntialiased.set("innerTexelSize", 2.f / 5.f / glm::vec2{innerWidth, innerHeight});
    mRoundedSolidShaderAntialiased.set("outerTexelSize", 2.f / 5.f / glm::vec2{width, height});
    drawRect(x, y, width, height);
    setFill(mCurrentFill);

}

void Render::drawRectBorderSide(float x, float y, float width, float height, float lineWidth, ASide s)
{
	uploadToShader();
	mTempVao.bind();

	auto doDraw = [&](ASide s)
	{
		glm::vec2 begin;
		glm::vec2 end;

		switch (s)
		{
		case S_NONE:
			return;
		case S_TOP:
			begin = { x, y };
			end = { x + width, y };
			break;
		case S_BOTTOM:
			end = { x, y + height };
			begin = { x + width, y + height };
			break;
		case S_LEFT:
			end = { x, y};
			begin = { x, y + height };
			break;
		case S_RIGHT:
			begin = { x + width, y};
			end = { x + width, y + height };
			break;
		}
		auto negativeNormal = -ASides::getNormalVector(s);
		auto tangent = glm::vec2{ negativeNormal.y, -negativeNormal.x };

		AVector<glm::vec2> temp = {
			{begin},
			{begin + (negativeNormal + tangent) * lineWidth},
			{end},
			{end + (negativeNormal - tangent) * lineWidth}
		};
		AVector<glm::vec3> result;
		result.reserve(temp.size());

		for (auto& e : temp)
		{
			result << glm::vec3(getTransform() * glm::vec4(e.x, e.y, 0, 1));
		}

		mTempVao.insert(0, result);
		mTempVao.indices({ 0, 1, 2, 2, 1, 3 });
		mTempVao.draw();
	};

	doDraw(s & S_LEFT);
	doDraw(s & S_RIGHT);
	doDraw(s & S_TOP);
	doDraw(s & S_BOTTOM);
}

void Render::drawRectBorder(float x, float y, float width, float height, float lineWidth)
{
	uploadToShader();
	mTempVao.bind();

	//rect.insert(0, getVerticesForRect(x + 0.25f + lineWidth * 0.5f, y + 0.25f + lineWidth * 0.5f, width - (0.25f + lineWidth * 0.5f), height - (0.75f + lineWidth * 0.5f)));

	const float lineDelta = 0.25f + lineWidth / 2.f;
	float w = x + width;
	float h = y + height;

	mTempVao.insert(0,
		AVector<glm::vec3>{
			glm::vec3(mTransform * glm::vec4{ x + lineWidth, y + lineDelta, 1, 1 }),
			glm::vec3(mTransform * glm::vec4{ w,             y + lineDelta, 1, 1 }),

			glm::vec3(mTransform * glm::vec4{ w - lineDelta, y + lineWidth, 1, 1 }),
			glm::vec3(mTransform * glm::vec4{ w - lineDelta, h            , 1, 1 }),

			glm::vec3(mTransform * glm::vec4{ w - lineWidth, h - lineDelta - 0.15f, 1, 1 }),
			glm::vec3(mTransform * glm::vec4{ x            , h - lineDelta - 0.15f, 1, 1 }),

			glm::vec3(mTransform * glm::vec4{ x + lineDelta, h - lineWidth - 0.15f, 1, 1 }),
			glm::vec3(mTransform * glm::vec4{ x + lineDelta, y            , 1, 1 }),
		});

	mTempVao.indices({ 0, 1, 2, 3, 4, 5, 6, 7});
	glLineWidth(lineWidth);
	mTempVao.draw(GL_LINES);
}

void Render::drawBoxShadow(float x, float y, float width, float height, float blurRadius, const AColor& color) {
    mBoxShadowShader.use();
    mBoxShadowShader.set("sigma", blurRadius / 2.f);
    mBoxShadowShader.set("lower", glm::vec2(x, y) + glm::vec2(width, height));
    mBoxShadowShader.set("upper", glm::vec2(x, y));
    mBoxShadowShader.set("transform", mTransform);
    mBoxShadowShader.set("color", mColor * color);

    mTempVao.bind();

    float w = x + width;
    float h = y + height;

    x -= blurRadius;
    y -= blurRadius;
    w += blurRadius;
    h += blurRadius;

    mTempVao.insert(0, AVector<glm::vec2>{
            glm::vec2{ x, h },
            glm::vec2{ w, h },
            glm::vec2{ x, y },
            glm::vec2{ w, y },
    });

    mTempVao.indices({ 0, 1, 2, 2, 1, 3 });
    mTempVao.draw();

    setFill(mCurrentFill);
}

void Render::setFill(Filling t)
{
    mCurrentFill = t;
	switch (t)
	{
	case FILL_SOLID:
		mSolidShader.use();
		break;

	case FILL_GRADIENT:
		mGradientShader.use();
		break;

	case FILL_TEXTURED:
		mTexturedShader.use();
		break;

	case FILL_SYMBOL:
		mSymbolShader.use();
		break;

	case FILL_SYMBOL_SUBPIXEL:
		mSymbolShaderSubPixel.use();
		break;

	case FILL_SOLID_TRANSFORM:
		mSolidTransformShader.use();
		break;
	}

	GL::Shader::currentShader()->set("color", mColor);
}

void Render::drawString(int x, int y, const AString& text, FontStyle& fs) {
	auto s = preRendererString(text, fs);
	drawString(x, y, s);
}
void Render::drawString(int x, int y, PrerendereredString& f) {
	auto img = f.fs.font->texturePackerOf(f.fs.size, f.fs.fontRendering);
	if (!img->getImage())
		return;

	auto width = img->getImage()->getWidth();
	if (width != f.side) {
		f.side = width;
		// PreRendereredString ��������� � �����������
		f = preRendererString(f.mText, f.fs);
	}


	if (f.fs.align == ALIGN_CENTER)
		x -= f.length / 2;
	else if (f.fs.align == ALIGN_RIGHT)
		x -= f.length;

	f.fs.font->textureOf(f.fs.size, f.fs.fontRendering)->bind();


	auto finalColor = mColor * f.fs.color;
	if (f.fs.fontRendering & FR_SUBPIXEL) {
		setFill(FILL_SYMBOL_SUBPIXEL);
		mSymbolShaderSubPixel.set("mat", glm::translate(mTransform, glm::vec3{x, y, 0}));

		mSymbolShaderSubPixel.set("color", glm::vec4(1, 1, 1, finalColor.a));
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		f.mVao->draw(GL_TRIANGLES);

		mSymbolShaderSubPixel.set("color", finalColor);
		glBlendFunc(GL_ONE, GL_ONE);
		f.mVao->draw(GL_TRIANGLES);

		// reset blending
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		setFill(FILL_SYMBOL);
		mSymbolShader.set("pos_x", x);
		mSymbolShader.set("pos_y", y);
        mSymbolShader.set("mat", glm::translate(mTransform, glm::vec3{x, y, 0}));
		mSymbolShader.set("color", finalColor);
		f.mVao->draw(GL_TRIANGLES);
	}
}



Render::PrerendereredString Render::preRendererString(const AString& text, FontStyle& fs) {
	static _<AFont> d = AFontManager::inst().getDefault();
	if (!fs.font)
		fs.font = d;
	float advance = 0;
	float advanceMax = 0;
	int advanceY = 0;


	AVector<glm::vec3> pos;
	AVector<glm::vec2> uvs;
	AVector<GLuint> indices;

	auto img = fs.font->texturePackerOf(fs.size, fs.fontRendering)->getImage();
	int prevWidth = -1;

	if (img) {
		prevWidth = img->getWidth();
	}

	size_t counter = 0;

	const bool hasKerning = fs.font->isHasKerning();

	for (auto i = text.begin(); i != text.end(); ++i, ++counter) {
		wchar_t c = *i;
		if (c == ' ') {
			advance += fs.size / 2.3f;
		}
		else if (c == '\n') {
            advanceMax = (glm::max)(advanceMax, advance);
			advance = 0;
			advanceY += fs.getLineHeight();
		}
		else {
			AFont::Character* ch = fs.font->getCharacter(c, fs.size, fs.fontRendering);
			if (!ch) {
				advance += fs.size / 2.3f;
				continue;
			}
			if ((advance >= 0 && advance <= 99999) /* || gui3d */) {
				unsigned int in = static_cast<unsigned int>(pos.size());
				indices.push_back(in);
				indices.push_back(in + 1);
				indices.push_back(in + 2);
				indices.push_back(in + 2);
				indices.push_back(in + 1);
				indices.push_back(in + 3);

				int posX = advance + ch->bearingX;

				pos.push_back(glm::vec3(posX, int(ch->advanceY) + int(ch->height) + advanceY, 0));
				pos.push_back(glm::vec3(posX + ch->width, int(ch->advanceY) + int(ch->height) + advanceY, 0));
				pos.push_back(glm::vec3(posX, int(ch->advanceY) + advanceY, 0));
				pos.push_back(glm::vec3(posX + ch->width, int(ch->advanceY) + advanceY, 0));

				uvs.push_back(glm::vec2(ch->uv->x, ch->uv->w));
				uvs.push_back(glm::vec2(ch->uv->z, ch->uv->w));
				uvs.push_back(glm::vec2(ch->uv->x, ch->uv->y));
				uvs.push_back(glm::vec2(ch->uv->z, ch->uv->y));
			}

			if (hasKerning) {
				auto next = std::next(i);
				if (next != text.end())
				{
					auto kerning = fs.font->getKerning(c, *next);
					advance += kerning.x;
				}
			}

			advance += ch->advanceX;
			advance = glm::floor(advance);
		}
	}
	auto vao = _new<GL::Vao>();
	{
		vao->insert(0, pos);
		vao->insert(1, uvs);
		vao->indices(indices);
	}

	if (prevWidth != -1 && fs.font->texturePackerOf(fs.size, fs.fontRendering)->getImage()->getWidth() != prevWidth) {
		return preRendererString(text, fs); // ������ ��������
	}
    advanceMax = (glm::max)(advanceMax, advance);

    //assert(advanceMax == fs.font->length(text, fs.size, fs.fontRendering));

	return { vao, fs, advanceMax, uint16_t(prevWidth), text };
}

void Render::uploadToShader()
{
	GL::Shader::currentShader()->set("color", mColor);
	GL::Shader::currentShader()->set("transform", mTransform);
}

glm::vec2 Render::getCurrentPos() {
    return glm::vec2(mTransform * glm::vec4(0, 0, 0, 1.f));
}

void Render::setGradientColors(const AColor& tl, const AColor& tr,
                               const AColor& bl, const AColor& br) {
    mGradientShader.use();
    mGradientShader.set("color_tl", tl);
    mGradientShader.set("color_tr", tr);
    mGradientShader.set("color_bl", bl);
    mGradientShader.set("color_br", br);
}

void Render::applyTextureRepeat() {
    if (Render::inst().getRepeat() == REPEAT_NONE) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
}

Render& Render::inst() {
    static Render r;
    return r;
}



