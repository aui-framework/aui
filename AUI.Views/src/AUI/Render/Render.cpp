#include "Render.h"

#include <glm/gtc/matrix_transform.hpp>
#include "AUI/Common/AColor.h"
#include "AFontManager.h"
#include "AUI/Common/ASide.h"
#include "AUI/Platform/AWindow.h"

Render::Render()
{
	mSolidShader.load(
		"attribute vec3 pos;"
		"void main(void) {gl_Position = vec4(pos, 1);}",
		"uniform vec4 color;"
		"void main(void) {gl_FragColor = color;}");
    mBoxShadowShader.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv * 2.f - vec2(1, 1);}",
        "varying vec2 pass_uv;"
        "uniform vec4 color;"
        "uniform vec2 size;"
        "void main(void) {"
            "gl_FragColor = vec4(1,0,0,1);"
            "vec2 tmp = abs(pass_uv);"
            "vec2 inv_size = 1.f - size;"
            "vec2 delta = (1.f - tmp) / inv_size / 2.f;"
            "gl_FragColor.a *= length(delta);"
         "}");

	mRoundedSolidShader.load(
		"attribute vec3 pos;"
        "attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv * 2.f - vec2(1, 1);}",
        "uniform vec2 size;"
        "varying vec2 pass_uv;"
		"void main(void) {"
            "vec2 tmp = abs(pass_uv);"
            "if ((tmp.x - 1) * (size.y) / (-size.x) < tmp.y - (1 - size.y) &&"
                "(pow(tmp.x - (1.f - size.x), 2.f) / pow(size.x, 2.f) +"
                 "pow(tmp.y - (1.f - size.y), 2.f) / pow(size.y, 2.f)) > 1.f) discard;"
        "}");

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
		"varying vec2 pass_uv;"
		"void main(void) {gl_FragColor = texture2D(tex, pass_uv);}",
		{"pos", "uv"});

	mSymbolShader.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"uniform mat4 mat;"
		"uniform int pos_x;"
		"uniform int pos_y;"

		"void main(void) {gl_Position = mat * vec4(pos + vec3(pos_x, pos_y, 0), 1); pass_uv = uv;}",
		"varying vec2 pass_uv;"
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"void main(void) {float sample = pow(texture2D(tex, pass_uv).r, 1.f / 1.2f); gl_FragColor = vec4(color.rgb, color.a * sample);}",
		{"pos", "uv"});

	mSymbolShaderSubPixel.load(
		"attribute vec3 pos;"
		"attribute vec2 uv;"
		"varying vec2 pass_uv;"
		"uniform mat4 mat;"
		"uniform int pos_x;"
		"uniform int pos_y;"

		"void main(void) {gl_Position = mat * vec4(pos + vec3(pos_x, pos_y, 0), 1); pass_uv = uv;}",
		"varying vec2 pass_uv;"
		"uniform sampler2D tex;"
		"uniform vec4 color;"
		"void main(void) {vec3 sample = pow(texture2D(tex, pass_uv).rgb, vec3(1.f / 1.2f)); gl_FragColor = vec4(sample * color.rgb * color.a, 1);}",
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

void Render::drawRoundedRect(float x, float y, float width, float height, float radius) {
    mRoundedSolidShader.use();
    mRoundedSolidShader.set("size", 2.f * radius / glm::vec2{width, height});
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
    mBoxShadowShader.set("size", blurRadius / glm::vec2(width, height));
    mBoxShadowShader.set("color", color);
    drawRect(x, y, width, height);
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
		mSymbolShaderSubPixel.set("pos_x", x);
		mSymbolShaderSubPixel.set("pos_y", y);
		mSymbolShaderSubPixel.set("mat", mTransform);

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
		mSymbolShader.set("mat", mTransform);
		mSymbolShader.set("color", finalColor);
		f.mVao->draw(GL_TRIANGLES);
	}
}



Render::PrerendereredString Render::preRendererString(const AString& text, FontStyle& fs) {
	static _<AFont> d = AFontManager::instance().getDefault();
	if (!fs.font)
		fs.font = d;
	float advance = 0;
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
		if (*i == ' ') {
			advance += fs.size / 2.3f;
		}
		else if (*i == '\n') {
			advance = 0;
			advanceY += fs.font->getAscenderHeight(fs.size) * (1.f + fs.lineSpacing);
		}
		else {
			AFont::Character* ch = fs.font->getCharacter(*i, fs.size, fs.fontRendering);
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
					auto kerning = fs.font->getKerning(*i, *next);
					advance += kerning.x;
				}
			}

			advance += ch->advanceX;
			//advance = glm::ceil(advance);
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
	return { vao, fs, fs.font->length(text, fs.size, fs.fontRendering), uint16_t(prevWidth), text };
}

void Render::uploadToShader()
{
	GL::Shader::currentShader()->set("color", mColor);
	GL::Shader::currentShader()->set("transform", mTransform);
}


