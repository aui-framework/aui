// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 11/19/2021.
//

#include "OpenGLRenderer.h"
#include "ShaderUniforms.h"
#include "AUI/Render/Render.h"
#include <AUI/Traits/callables.h>
#include <AUI/Platform/AFontManager.h>
#include <AUI/GL/Vbo.h>
#include <AUI/GL/State.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUISL/Generated/basic.vsh.glsl120.h>
#include <AUISL/Generated/basic_uv.vsh.glsl120.h>
#include <AUISL/Generated/solid.fsh.glsl120.h>
#include <AUISL/Generated/shadow.fsh.glsl120.h>
#include <AUISL/Generated/rounded.fsh.glsl120.h>


class OpenGLTexture2D: public ITexture {
private:
    gl::Texture2D mTexture;

public:
    void setImage(const _<AImage>& image) override {
        mTexture.tex2D(*image);
    }

    void bind() {
        mTexture.bind();
    }
};

static constexpr GLuint RECT_INDICES[] = {0, 1, 2, 2, 1, 3 };


template<typename Brush>
struct UnsupportedBrushHelper {
    void operator()(const Brush& brush) const {
        assert(("this brush is unsupported"));
    }
};

struct GradientShaderHelper {
    gl::Program& shader;

    GradientShaderHelper(gl::Program& shader) : shader(shader) {}

    void operator()(const ALinearGradientBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor());
        shader.set(aui::ShaderUniforms::COLOR_TL, brush.topLeftColor);
        shader.set(aui::ShaderUniforms::COLOR_TR, brush.topRightColor);
        shader.set(aui::ShaderUniforms::COLOR_BL, brush.bottomLeftColor);
        shader.set(aui::ShaderUniforms::COLOR_BR, brush.bottomRightColor);
    }
};

struct SolidShaderHelper {
    gl::Program& shader;

    SolidShaderHelper(gl::Program& shader) : shader(shader) {}

    void operator()(const ASolidBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::SL_UNIFORM_COLOR, Render::getColor() * brush.solidColor);
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor() * brush.solidColor);
    }
};

struct CustomShaderHelper {

    CustomShaderHelper() {}

    void operator()(const ACustomShaderBrush& brush) const {
    }
};

struct TexturedShaderHelper {
    gl::Program& shader;
    gl::Vao& tempVao;

    TexturedShaderHelper(gl::Program& shader, gl::Vao& tempVao) : shader(shader), tempVao(tempVao) {}

    void operator()(const ATexturedBrush& brush) const {
        shader.use();
        switch (brush.imageRendering) {
            case ImageRendering::PIXELATED:
                gl::Texture2D::setupNearest();
                break;
            case ImageRendering::SMOOTH:
                gl::Texture2D::setupLinear();
                break;
        }
        shader.set(aui::ShaderUniforms::COLOR, Render::getColor());
        glm::vec2 uv1 = brush.uv1 ? *brush.uv1 : glm::vec2{0, 0};
        glm::vec2 uv2 = brush.uv2 ? *brush.uv2 : glm::vec2{1, 1};
        tempVao.bind();
        glEnableVertexAttribArray(1);
        const glm::vec2 uvs[] = {
            {uv1.x, uv2.y},
            {uv2.x, uv2.y},
            {uv1.x, uv1.y},
            {uv2.x, uv1.y},
        };
        tempVao.insert(1, uvs);
        _cast<OpenGLTexture2D>(brush.texture)->bind();
    }
};


std::string put_if(bool value, const char* str) { if (value) return str; return ""; }


template<typename C>
concept AuiSLShader = requires(C&& c) {
    { C::code() } -> std::same_as<const char*>;
    C::setup();
};

template<AuiSLShader Vertex, AuiSLShader Fragment>
inline void useAuislShader(gl::Program& out) {
    out.loadRaw(Vertex::code(), Fragment::code());
    Vertex::setup();
    Fragment::setup();
    out.compile();
}

OpenGLRenderer::OpenGLRenderer() {

    useAuislShader<aui::sl_gen::basic::vsh::glsl120::Shader,
                   aui::sl_gen::solid::fsh::glsl120::Shader>(mSolidShader);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::shadow::fsh::glsl120::Shader>(mBoxShadowShader);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::rounded::fsh::glsl120::Shader>(mRoundedSolidShader);

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
        // without antialiasing rounded borders look poorly. fix it
        /*
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
                "}");*/
        auto produceRoundedAntialiasedShader = [](gl::Program& shader, const AString& uniforms, const AString& color, bool isBorder) {
            shader.load(
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
                    + uniforms +
                    "uniform vec4 color;"
                    "varying vec2 pass_uv;"
                    "bool is_outside(vec2 tmp, vec2 size) {"
                    "if (tmp.x >= 1.0 || tmp.y >= 1.0) return true;"
                    "return (tmp.x - 1.0) * (size.y) / (-size.x) <= tmp.y - (1.0 - size.y) &&"
                    "(pow(tmp.x - (1.0 - size.x), 2.0) / pow(size.x, 2.0) +"
                    "pow(tmp.y - (1.0 - size.y), 2.0) / pow(size.y, 2.0)) >= 1.0;"
                    "}"
                    "void main(void) {"
                    "vec2 outer_uv = abs(pass_uv);"
                    "vec2 inner_uv = outer_uv * outer_to_inner;"
                    "float alpha = 1.0;"
                    "ivec2 i;"
                    "for (i.x = -2; i.x <= 2; ++i.x) {"
                    "for (i.y = -2; i.y <= 2; ++i.y) {"
                    "alpha -= (" + put_if(isBorder, "is_outside(inner_uv + innerTexelSize * vec2(i), innerSize)"
                    " == ") + "is_outside(outer_uv + outerTexelSize * vec2(i), outerSize)"
                    ") ? (1.0 / 25.0) : 0.0;"
                    "}"
                    "}"
                    + color +
                    "gl_FragColor = vec4(fcolor.rgb, fcolor.a * alpha);"
                    "}", { "pos", "uv" });
        };
        /*
        produceRoundedAntialiasedShader(mRoundedSolidShaderAntialiased,
                                        {},
                                        "vec4 fcolor = color;",
                                        false);

        produceRoundedAntialiasedShader(mRoundedSolidShaderAntialiasedBorder,
                                        {},
                                        "vec4 fcolor = color;",
                                        true);

        produceRoundedAntialiasedShader(mRoundedGradientShaderAntialiased,
                                        "uniform vec4 color_tl;"
                                        "uniform vec4 color_tr;"
                                        "uniform vec4 color_bl;"
                                        "uniform vec4 color_br;",
                                        "vec4 fcolor = mix(mix(color_tl, color_tr, pass_uv.x), mix(color_bl, color_br, pass_uv.x), pass_uv.y) * color;",
                                        false);*/
    }

    mTexturedShader.load(
            "attribute vec3 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_Position = vec4(pos, 1); pass_uv = uv;}",
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "varying vec2 pass_uv;"
            "void main(void) {gl_FragColor = texture2D(tex, pass_uv) * color; if (gl_FragColor.a < 0.01) discard;}",
            {"pos", "uv"});

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
            "attribute vec2 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "uniform mat4 mat;"
            "uniform float uv_scale;"

            "void main(void) {gl_Position = mat * vec4(pos, 1, 1); pass_uv = uv * uv_scale;}",
            "varying vec2 pass_uv;"
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "void main(void) {float sample = texture2D(tex, pass_uv).r; gl_FragColor = vec4(color.rgb, color.a * sample);}",
            {"pos", "uv"});

    mSymbolShaderSubPixel.load(
            "attribute vec2 pos;"
            "attribute vec2 uv;"
            "varying vec2 pass_uv;"
            "uniform mat4 mat;"
            "uniform float uv_scale;"

            "void main(void) {gl_Position = mat * vec4(pos, 1, 1); pass_uv = uv * uv_scale;}",
            "varying vec2 pass_uv;"
            "uniform sampler2D tex;"
            "uniform vec4 color;"
            "void main(void) {vec3 sample = texture2D(tex, pass_uv).rgb; gl_FragColor = vec4(sample * color.rgb * color.a, 1);}",
            {"pos", "uv"});

    mTempVao.bind();

    const glm::vec2 uvs[] = {
        {0, 1},
        {1, 1},
        {0, 0},
        {1, 0}
    };
    mTempVao.insert(1, uvs);
}

glm::mat4 OpenGLRenderer::getProjectionMatrix() const {
    return glm::ortho(0.f, static_cast<float>(mWindow->getWidth()), static_cast<float>(mWindow->getHeight()), 0.f);
}

void OpenGLRenderer::uploadToShaderCommon() {
    gl::Program::currentShader()->set(aui::ShaderUniforms::TRANSFORM, mTransform);
}

AVector<glm::vec3> OpenGLRenderer::getVerticesForRect(glm::vec2 position, glm::vec2 size)
{
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    return
            {
                    glm::vec3(glm::vec4{ x, h, 1, 1 }),
                    glm::vec3(glm::vec4{ w, h, 1, 1 }),
                    glm::vec3(glm::vec4{ x, y, 1, 1 }),
                    glm::vec3(glm::vec4{ w, y, 1, 1 }),
            };
}
void OpenGLRenderer::drawRect(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mGradientShader),
            TexturedShaderHelper(mTexturedShader, mTempVao),
            SolidShaderHelper(mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();

    drawRectImpl(position, size);

    endDraw(brush);
}

void OpenGLRenderer::drawRectImpl(glm::vec2 position, glm::vec2 size) {
    mTempVao.bind();

    mTempVao.insert(0, getVerticesForRect(position, size));
    mTempVao.indices(RECT_INDICES);
    mTempVao.drawElements();
}

void OpenGLRenderer::drawRoundedRect(const ABrush& brush,
                                     glm::vec2 position,
                                     glm::vec2 size,
                                     float radius) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mRoundedGradientShader),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mRoundedSolidShader),
            CustomShaderHelper{},
    }, brush);

    uploadToShaderCommon();

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    gl::Program::currentShader()->set(aui::ShaderUniforms::INNER_TEXEL_SIZE, glm::vec2{0, 0});
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TEXEL_SIZE, 2.f / 5.f / size);
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, glm::vec2{0});

    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawRectBorder(const ABrush& brush,
                                    glm::vec2 position,
                                    glm::vec2 size,
                                    float lineWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();
    mTempVao.bind();

    //rect.insert(0, getVerticesForRect(x + 0.25f + lineWidth * 0.5f, y + 0.25f + lineWidth * 0.5f, width - (0.25f + lineWidth * 0.5f), height - (0.75f + lineWidth * 0.5f)));

    const float lineDelta = 0.25f + lineWidth / 2.f;
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

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

    constexpr GLuint INDICES[] = { 0, 1, 2, 3, 4, 5, 6, 7};
    mTempVao.indices(INDICES);
    glLineWidth(lineWidth);
    mTempVao.drawElements(GL_LINES);
    endDraw(brush);
}

void OpenGLRenderer::drawRectBorder(const ABrush& brush,
                                    glm::vec2 position,
                                    glm::vec2 size,
                                    float radius,
                                    int borderWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(mRoundedSolidShaderBorder),
            CustomShaderHelper{},
    }, brush);

    glm::vec2 innerSize = { size.x - borderWidth * 2,
                            size.y - borderWidth * 2 };

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    gl::Program::currentShader()->set(aui::ShaderUniforms::INNER_SIZE, 2.f * (radius - borderWidth) / innerSize);
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, size / innerSize);

    gl::Program::currentShader()->set(aui::ShaderUniforms::INNER_TEXEL_SIZE, 2.f / 5.f / innerSize);
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TEXEL_SIZE, 2.f / 5.f / size);

    drawRectImpl(position, size);
    endDraw(brush);
}

void OpenGLRenderer::drawBoxShadow(glm::vec2 position,
                                   glm::vec2 size,
                                   float blurRadius,
                                   const AColor& color) {
    mBoxShadowShader.use();
    mBoxShadowShader.set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, blurRadius / 2.f);
    mBoxShadowShader.set(aui::ShaderUniforms::SL_UNIFORM_LOWER, position + size);
    mBoxShadowShader.set(aui::ShaderUniforms::SL_UNIFORM_UPPER, position);
    mBoxShadowShader.set(aui::ShaderUniforms::SL_UNIFORM_TRANSFORM, mTransform);
    mBoxShadowShader.set(aui::ShaderUniforms::SL_UNIFORM_COLOR, mColor * color);

    mTempVao.bind();

    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    x -= blurRadius;
    y -= blurRadius;
    w += blurRadius;
    h += blurRadius;

    const glm::vec2 uvs[] = {
        { x, h },
        { w, h },
        { x, y },
        { w, y },
    };
    mTempVao.insert(0, uvs);

    mTempVao.indices(RECT_INDICES);
    mTempVao.drawElements();
}

void OpenGLRenderer::drawString(glm::vec2 position,
                                const AString& string,
                                const AFontStyle& fs) {
    prerenderString(position, string, fs)->draw();
}


void OpenGLRenderer::endDraw(const ABrush& brush) {
    if (std::holds_alternative<ATexturedBrush>(brush)) {
    }
}

void OpenGLRenderer::setBlending(Blending blending) {
    switch (blending) {
        case Blending::NORMAL:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case Blending::INVERSE_DST:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            break;
    }
}

class OpenGLPrerenderedString: public IRenderer::IPrerenderedString {
public:
    struct Vertex {
        glm::vec2 position;
        glm::vec2 uv;
    };
    OpenGLRenderer* mRenderer;
    gl::VertexBuffer mVertexBuffer;
    gl::IndexBuffer mIndexBuffer;
    int mTextWidth;
    int mTextHeight;
    OpenGLRenderer::FontEntryData* mEntryData;
    AColor mColor;
    FontRendering mFontRendering;

    OpenGLPrerenderedString(OpenGLRenderer* renderer,
                            gl::VertexBuffer vertexBuffer,
                            gl::IndexBuffer indexBuffer,
                            int textWidth,
                            int textHeight,
                            OpenGLRenderer::FontEntryData* entryData,
                            AColor color,
                            FontRendering fontRendering):
            mRenderer(renderer),
            mVertexBuffer(std::move(vertexBuffer)),
            mIndexBuffer(std::move(indexBuffer)),
            mTextWidth(textWidth),
            mTextHeight(textHeight),
            mEntryData(entryData),
            mColor(color),
            mFontRendering(fontRendering)
    {}


    void draw() override {
        if (mIndexBuffer.count() == 0) return;

        // TODO get rid of vao
        if (mRenderer->isVaoAvailable()) {
            static GLuint g = [] {
                GLuint a;
                glGenVertexArrays(1, &a);
                return a;
            }();
            gl::State::bindVertexArray(g);
        }

        decltype(auto) img = mEntryData->texturePacker.getImage();
        if (!img)
            return;

        auto width = img->width();

        float uvScale = 1.f / float(width);

        if (mEntryData->isTextureInvalid) {
            mEntryData->texture.tex2D(*img);
            mEntryData->isTextureInvalid = false;
        } else {
            mEntryData->texture.bind();
        }
        gl::Texture2D::setupNearest();

        mVertexBuffer.bind();

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex), reinterpret_cast<const void*>(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex), reinterpret_cast<const void*>(sizeof(glm::vec2)));

        auto finalColor = Render::getColor() * mColor;
        if (mFontRendering == FontRendering::SUBPIXEL) {
            mRenderer->mSymbolShaderSubPixel.use();
            mRenderer->mSymbolShaderSubPixel.set(aui::ShaderUniforms::UV_SCALE, uvScale);
            mRenderer->mSymbolShaderSubPixel.set(aui::ShaderUniforms::MAT, mRenderer->getTransform());
            mRenderer->mSymbolShaderSubPixel.set(aui::ShaderUniforms::COLOR, glm::vec4(1, 1, 1, finalColor.a));
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
            mIndexBuffer.draw(GL_TRIANGLES);

            mRenderer->mSymbolShaderSubPixel.set(aui::ShaderUniforms::COLOR, finalColor);
            glBlendFunc(GL_ONE, GL_ONE);
            mIndexBuffer.draw(GL_TRIANGLES);

            // reset blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            mRenderer->mSymbolShader.use();
            mRenderer->mSymbolShader.set(aui::ShaderUniforms::UV_SCALE, uvScale);
            mRenderer->mSymbolShader.set(aui::ShaderUniforms::MAT, mRenderer->getTransform());
            mRenderer->mSymbolShader.set(aui::ShaderUniforms::COLOR, finalColor);
            mIndexBuffer.draw(GL_TRIANGLES);
        }
        glDisableVertexAttribArray(1);
    }

    int getWidth() override {
        return mTextWidth;
    }

    int getHeight() override {
        return mTextHeight;
    }
};

class OpenGLMultiStringCanvas: public IRenderer::IMultiStringCanvas {
private:
    AVector<OpenGLPrerenderedString::Vertex> mVertices;
    OpenGLRenderer* mRenderer;
    AFontStyle mFontStyle;
    OpenGLRenderer::FontEntryData* mEntryData;
    int mAdvanceX = 0;
    int mAdvanceY = 0;

public:
    OpenGLMultiStringCanvas(OpenGLRenderer* renderer, const AFontStyle& fontStyle):
            mRenderer(renderer),
            mFontStyle(fontStyle),
            mEntryData(renderer->getFontEntryData(fontStyle)) {
        mVertices.reserve(1000);
    }

    void addString(const glm::ivec2& position, const AString& text) noexcept override {
        mVertices.reserve(mVertices.capacity() + text.length() * 4);
        auto& font = mFontStyle.font;
        auto& texturePacker = mEntryData->texturePacker;
        auto fe = mFontStyle.getFontEntry();

        const bool hasKerning = font->isHasKerning();

        int advanceX = position.x;
        int advanceY = position.y - mFontStyle.font->getDescenderHeight(mFontStyle.size);
        size_t counter = 0;
        int advance = advanceX;
        for (auto i = text.begin(); i != text.end(); ++i, ++counter) {
            wchar_t c = *i;
            if (c == ' ') {
                notifySymbolAdded({glm::ivec2{advance, advanceY}});
                advance += mFontStyle.getSpaceWidth();
            }
            else if (c == '\n') {
                advanceX = (glm::max)(advanceX, advance);
                advance = position.x;
                advanceY += mFontStyle.getLineHeight();
                nextLine();
            }
            else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    advance += mFontStyle.getSpaceWidth();
                    continue;
                }
                if ((advance >= 0 && advance <= 99999) /* || gui3d */) {

                    int posX = advance + ch.bearingX;
                    int width = ch.image->width();
                    int height = ch.image->height();

                    glm::vec4 uv;

                    if (ch.rendererData == nullptr) {
                        uv = texturePacker.insert(*ch.image);

                        const float BIAS = 0.1f;
                        uv.x += BIAS;
                        uv.y += BIAS;
                        uv.z -= BIAS;
                        uv.w -= BIAS;
                        mRenderer->mCharData.push_back(OpenGLRenderer::CharacterData{uv});
                        ch.rendererData = &mRenderer->mCharData.last();
                        mEntryData->isTextureInvalid = true;
                    } else {
                        uv = reinterpret_cast<OpenGLRenderer::CharacterData*>(ch.rendererData)->uv;
                    }

                    notifySymbolAdded({glm::ivec2{posX, ch.advanceY + advanceY}});
                    mVertices.push_back({ glm::vec2(posX, ch.advanceY + height + advanceY),
                                          glm::vec2(uv.x, uv.w) });
                    mVertices.push_back({ glm::vec2(posX + width, ch.advanceY + height + advanceY),
                                          glm::vec2(uv.z, uv.w) });
                    mVertices.push_back({ glm::vec2(posX, ch.advanceY + advanceY),
                                          glm::vec2(uv.x, uv.y) });
                    mVertices.push_back({ glm::vec2(posX + width, ch.advanceY + advanceY),
                                          glm::vec2(uv.z, uv.y) });

                }

                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != text.end())
                    {
                        auto kerning = font->getKerning(c, *next);
                        advance += kerning.x;
                    }
                }

                advance += ch.advanceX;
                advance = glm::floor(advance);
            }
        }

        mAdvanceX = (glm::max)(mAdvanceX, (glm::max)(advanceX, advance));
        mAdvanceY = advanceY + mFontStyle.getLineHeight();
    }

    _<IRenderer::IPrerenderedString> finalize() noexcept override {
        gl::VertexBuffer vertexBuffer;
        vertexBuffer.set(mVertices);

        // build indices
        AVector<GLuint> indices;
        indices.reserve(mVertices.size() / 4 * 6);
        for (unsigned i = 0; i < mVertices.size() / 4; ++i) {
            indices.push_back(i * 4);
            indices.push_back(i * 4 + 1);
            indices.push_back(i * 4 + 2);
            indices.push_back(i * 4 + 2);
            indices.push_back(i * 4 + 1);
            indices.push_back(i * 4 + 3);
        }
        gl::IndexBuffer indexBuffer;
        indexBuffer.set(indices);

        return _new<OpenGLPrerenderedString>(mRenderer,
                                             std::move(vertexBuffer),
                                             std::move(indexBuffer),
                                             mAdvanceX,
                                             mAdvanceY,
                                             mEntryData,
                                             mFontStyle.color,
                                             mFontStyle.fontRendering);
    }

    ~OpenGLMultiStringCanvas() override = default;
};

_<IRenderer::IPrerenderedString> OpenGLRenderer::prerenderString(glm::vec2 position,
                                                                 const AString& text,
                                                                 const AFontStyle& fs) {
    if (text.empty()) return nullptr;

    OpenGLMultiStringCanvas c(this, fs);
    c.addString(position, text);

    return c.finalize();
}

OpenGLRenderer::FontEntryData* OpenGLRenderer::getFontEntryData(const AFontStyle& fontStyle) {
    auto fe = fontStyle.getFontEntry();
    FontEntryData* entryData;
    if (fe.second.rendererData == nullptr) {
        mFontEntryData.emplace_back();
        fe.second.rendererData = entryData = &mFontEntryData.last();
    } else {
        entryData = reinterpret_cast<FontEntryData*>(fe.second.rendererData);
    }
    return entryData;
}

ITexture* OpenGLRenderer::createNewTexture() {
    return new OpenGLTexture2D;
}

_<IRenderer::IMultiStringCanvas> OpenGLRenderer::newMultiStringCanvas(const AFontStyle& style) {
    return _new<OpenGLMultiStringCanvas>(this, style);
}

bool OpenGLRenderer::isVaoAvailable() {
    return glBindVertexArray != nullptr;
}

void OpenGLRenderer::pushMaskBefore() {
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);
}

void OpenGLRenderer::pushMaskAfter() {
    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, ++mStencilDepth, 0xff);
}

void OpenGLRenderer::popMaskBefore() {
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);
}

void OpenGLRenderer::popMaskAfter() {
    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, --mStencilDepth, 0xff);
}

void OpenGLRenderer::drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mGradientShader),
            TexturedShaderHelper(mTexturedShader, mTempVao),
            SolidShaderHelper(mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();

    mTempVao.bind();

    const glm::vec4 positions[] = {
        mTransform * glm::vec4(p1, 0, 1),
        mTransform * glm::vec4(p2, 0, 1),
    };
    mTempVao.insert(0, positions);
    mTempVao.drawArrays(GL_LINES, 2);

    endDraw(brush);
}

void OpenGLRenderer::drawLines(const ABrush& brush, AArrayView<glm::vec2> points) {
    if (points.size() < 2) return;
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mGradientShader),
            TexturedShaderHelper(mTexturedShader, mTempVao),
            SolidShaderHelper(mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();

    mTempVao.bind();

    AVector<glm::vec4> positions;
    positions.reserve(points.size());

    for (const auto& point : points) {
        positions << mTransform * glm::vec4(point, 0, 1);
    }

    mTempVao.insert(0, positions);
    mTempVao.drawArrays(GL_LINE_STRIP, points.size());

    endDraw(brush);
}

void OpenGLRenderer::drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(mGradientShader),
            TexturedShaderHelper(mTexturedShader, mTempVao),
            SolidShaderHelper(mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();

    mTempVao.bind();

    AVector<glm::vec4> positions;
    positions.reserve(points.size() * 2);

    for (const auto& [p1, p2] : points) {
        positions << mTransform * glm::vec4(p1, 0, 1);
        positions << mTransform * glm::vec4(p2, 0, 1);
    }

    mTempVao.insert(0, positions);
    mTempVao.drawArrays(GL_LINES, positions.size());

    endDraw(brush);
}

