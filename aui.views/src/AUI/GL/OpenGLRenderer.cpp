// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <range/v3/view.hpp>
#include "OpenGLRenderer.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/GL/Framebuffer.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/GL/GLEnums.h"
#include "AUI/GL/Program.h"
#include "AUI/GL/Texture2D.h"
#include "AUI/GL/Vao.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Render/ABorderStyle.h"
#include "AUI/Render/Brush/Gradient.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Util/AAngleRadians.h"
#include "AUI/Util/AArrayView.h"
#include "ShaderUniforms.h"
#include "AUI/Render/ARender.h"
#include "glm/fwd.hpp"
#include <AUI/Traits/callables.h>
#include <AUI/Platform/AFontManager.h>
#include <AUI/GL/Vbo.h>
#include <AUI/GL/State.h>
#include <AUI/GL/RenderTarget/RenderbufferRenderTarget.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Logging/ALogger.h>
#include <AUISL/Generated/basic.vsh.glsl120.h>
#include <AUISL/Generated/basic_uv.vsh.glsl120.h>
#include <AUISL/Generated/shadow.fsh.glsl120.h>
#include <AUISL/Generated/shadow_inner.fsh.glsl120.h>
#include <AUISL/Generated/rect_solid.fsh.glsl120.h>
#include <AUISL/Generated/rect_solid_rounded.fsh.glsl120.h>
#include <AUISL/Generated/rect_gradient.fsh.glsl120.h>
#include <AUISL/Generated/rect_gradient_rounded.fsh.glsl120.h>
#include <AUISL/Generated/rect_textured.fsh.glsl120.h>
#include <AUISL/Generated/border_rounded.fsh.glsl120.h>
#include <AUISL/Generated/symbol.vsh.glsl120.h>
#include <AUISL/Generated/symbol.fsh.glsl120.h>
#include <AUISL/Generated/symbol_sub.fsh.glsl120.h>
#include <AUISL/Generated/line_solid_dashed.fsh.glsl120.h>
#include <AUISL/Generated/square_sector.fsh.glsl120.h>
#include <glm/gtx/matrix_transform_2d.hpp>

static constexpr auto LOG_TAG = "OpenGLRenderer";

class OpenGLTexture2D: public ITexture {
private:
    gl::Texture2D mTexture;

public:
    void setImage(AImageView image) override {
        mTexture.tex2D(image);
    }

    void bind() {
        mTexture.bind();
    }

    gl::Texture2D& texture() noexcept {
        return mTexture;
    }
};

/**
 * @brief UV bias used to perfectly align texture coordinates to the pixel grid, avoiding unwanted repeat clipping.
 */
static constexpr auto UV_BIAS = 0.001f;


namespace {

template<typename Brush>
struct UnsupportedBrushHelper {
    void operator()(const Brush& brush) const {
        AUI_ASSERT(("this brush is unsupported"));
    }
};

struct GradientShaderHelper {
    OpenGLRenderer& renderer;
    gl::Program& shader;
    gl::Texture2D& tex;

    GradientShaderHelper(OpenGLRenderer& renderer, gl::Program& shader, gl::Texture2D& tex) : renderer(renderer), shader(shader), tex(tex) {}

    void operator()(const ALinearGradientBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, ARender::getColor());
        aui::render::brush::gradient::Helper h(brush);
        shader.set(aui::ShaderUniforms::GRADIENT_MAT_UV, h.matrix);

        if (h.colors.size() == 2) {
            // simple gradient can be used
            shader.set(aui::ShaderUniforms::COLOR1, glm::vec4(h.colors[0]) / 255.f);
            shader.set(aui::ShaderUniforms::COLOR2, glm::vec4(h.colors[1]) / 255.f);
        } else {
            // complex gradient needs a texture
            // tex.tex2D(h.gradientMap());

            // TODO complex shader is broken, use simple shader instead with first and last color
            shader.set(aui::ShaderUniforms::COLOR1, glm::vec4(h.colors.first()) / 255.f);
            shader.set(aui::ShaderUniforms::COLOR2, glm::vec4(h.colors.last()) / 255.f);
        }

        renderer.identityUv();
    }
};

struct SolidShaderHelper {
    gl::Program& shader;

    SolidShaderHelper(gl::Program& shader) : shader(shader) {}

    void operator()(const ASolidBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, ARender::getColor() * brush.solidColor);
    }
};

struct CustomShaderHelper {

    CustomShaderHelper() {}

    void operator()(const ACustomShaderBrush& brush) const {
    }
};

struct TexturedShaderHelper {
    OpenGLRenderer& renderer;
    gl::Program& shader;
    gl::Vao& tempVao;

    TexturedShaderHelper(OpenGLRenderer& renderer, gl::Program& shader, gl::Vao& tempVao) : renderer(renderer), shader(shader), tempVao(tempVao) {}

    void operator()(const ATexturedBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, ARender::getColor());
        if (brush.uv1 || brush.uv2) {
            glm::vec2 uv1 = brush.uv1.valueOr(glm::vec2{0, 0});
            glm::vec2 uv2 = brush.uv2.valueOr(glm::vec2{1, 1});

            const glm::vec2 uvs[] = {
                {uv1.x, uv2.y},
                {uv2.x, uv2.y},
                {uv1.x, uv1.y},
                {uv2.x, uv1.y},
            };
            tempVao.insert(1, AArrayView(uvs), "TexturedShaderHelper");
        } else {
            renderer.identityUv();
        } 

        auto tex = _cast<OpenGLTexture2D>(brush.texture);
        tex->bind();
        switch (brush.imageRendering) {
            case ImageRendering::PIXELATED:
                tex->texture().setupNearest();
                break;
            case ImageRendering::SMOOTH:
                tex->texture().setupLinear();
                break;
        }
    }
};


template<typename C>
concept AuiSLShader = requires(C&& c) {
    { C::code() } -> std::same_as<const char*>;
    C::setup(0);
};

template<AuiSLShader Vertex, AuiSLShader Fragment>
inline void useAuislShader(AOptional<gl::Program>& out) {
    out.emplace();
    out->loadRaw(Vertex::code(), Fragment::code());
    Vertex::setup(out->handle());
    Fragment::setup(out->handle());
    out->compile();
}
}

OpenGLRenderer::OpenGLRenderer() {
    gl::setupDebug();
    ALogger::info(LOG_TAG) << "GL_VERSION = " << ((const char*) glGetString(GL_VERSION));
    ALogger::info(LOG_TAG) << "GL_VENDOR = " << ((const char*) glGetString(GL_VENDOR));
    ALogger::info(LOG_TAG) << "GL_RENDERER = " << ((const char*) glGetString(GL_RENDERER));
    ALogger::info(LOG_TAG) << "GL_EXTENSIONS = " << [] {
        if (auto ext = ((const char*) glGetString(GL_EXTENSIONS))) {
            return ext;
        } else {
            return "null";
        }}();
    mGradientTexture.bind();
    mGradientTexture.setupLinear();
    mGradientTexture.setupClampToEdge();
    useAuislShader<aui::sl_gen::basic::vsh::glsl120::Shader,
                   aui::sl_gen::rect_solid::fsh::glsl120::Shader>(mSolidShader);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::shadow::fsh::glsl120::Shader>(mBoxShadowShader);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::shadow_inner::fsh::glsl120::Shader>(mBoxShadowInnerShader);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::rect_solid_rounded::fsh::glsl120::Shader>(mRoundedSolidShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::border_rounded::fsh::glsl120::Shader>(mRoundedSolidShaderBorder);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::rect_gradient::fsh::glsl120::Shader>(mGradientShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::rect_gradient_rounded::fsh::glsl120::Shader>(mRoundedGradientShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::rect_textured::fsh::glsl120::Shader>(mTexturedShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::square_sector::fsh::glsl120::Shader>(mSquareSectorShader);

    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader,
                   aui::sl_gen::symbol::fsh::glsl120::Shader>(mSymbolShader);
    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader,
                   aui::sl_gen::symbol_sub::fsh::glsl120::Shader>(mSymbolShaderSubPixel);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
                   aui::sl_gen::line_solid_dashed::fsh::glsl120::Shader>(mLineSolidDashedShader);

    {
        constexpr GLuint INDICES[] = {0, 1, 2, 2, 1, 3 };
        mRectangleVao.indices(INDICES);
    }
    {
        constexpr GLuint INDICES[] = { 0, 1, 2, 3, 4, 5, 6, 7};
        mBorderVao.indices(INDICES);
    }
}

glm::mat4 OpenGLRenderer::getProjectionMatrix() const {
    return glm::ortho(0.0f, static_cast<float>(mWindow->getWidth()) - 0.0f, static_cast<float>(mWindow->getHeight()) - 0.0f, 0.0f, -1.f, 1.f);
}

void OpenGLRenderer::uploadToShaderCommon() {
    gl::Program::currentShader()->set(aui::ShaderUniforms::TRANSFORM, mTransform);
}

std::array<glm::vec2, 4> OpenGLRenderer::getVerticesForRect(glm::vec2 position, glm::vec2 size)
{
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    auto apply = [&](glm::vec4 v) {
        auto result = mTransform * v;
        return glm::vec3(result) / result.w;
    };

    return
            {
                    glm::vec2{ x, h, },
                    glm::vec2{ w, h, },
                    glm::vec2{ x, y, },
                    glm::vec2{ w, y, },
            };
}
void OpenGLRenderer::drawRect(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
            TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
            SolidShaderHelper(*mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();

    drawRectImpl(position, size);
}

void OpenGLRenderer::drawRectImpl(glm::vec2 position, glm::vec2 size) {
    mRectangleVao.bind();

    mRectangleVao.insert(0, AArrayView(getVerticesForRect(position, size)), "drawRectImpl");

    mRectangleVao.drawElements();
}

void OpenGLRenderer::identityUv() {
    const glm::vec2 uvs[] = {
        {0, 1},
        {1, 1},
        {0, 0},
        {1, 0}
    };
    mRectangleVao.insertIfKeyMismatches(1, AArrayView(uvs), "identityUv");
}

void OpenGLRenderer::drawRoundedRect(const ABrush& brush,
                                     glm::vec2 position,
                                     glm::vec2 size,
                                     float radius) {
    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(*this, *mRoundedGradientShader, mGradientTexture),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(*mRoundedSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();
    identityUv();

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    drawRectImpl(position, size);
}

void OpenGLRenderer::drawRectBorder(const ABrush& brush,
                                    glm::vec2 position,
                                    glm::vec2 size,
                                    float lineWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(*mSolidShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();
    identityUv();

    //rect.insert(0, getVerticesForRect(x + 0.25f + lineWidth * 0.5f, y + 0.25f + lineWidth * 0.5f, width - (0.25f + lineWidth * 0.5f), height - (0.75f + lineWidth * 0.5f)));

    const float lineDelta = 0.25f + lineWidth / 2.f;
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    mRectangleVao.insert(0,
                    AArrayView(std::array<glm::vec3, 8>{
                            glm::vec3(glm::vec4{ x + lineWidth, y + lineDelta, 1, 1 }),
                            glm::vec3(glm::vec4{ w,             y + lineDelta, 1, 1 }),

                            glm::vec3(glm::vec4{ w - lineDelta, y + lineWidth, 1, 1 }),
                            glm::vec3(glm::vec4{ w - lineDelta, h            , 1, 1 }),

                            glm::vec3(glm::vec4{ w - lineWidth, h - lineDelta - 0.15f, 1, 1 }),
                            glm::vec3(glm::vec4{ x            , h - lineDelta - 0.15f, 1, 1 }),

                            glm::vec3(glm::vec4{ x + lineDelta, h - lineWidth - 0.15f, 1, 1 }),
                            glm::vec3(glm::vec4{ x + lineDelta, y            , 1, 1 }),
                    }), "drawRectBorder");

    glLineWidth(lineWidth);
    mRectangleVao.drawElements(GL_LINES);
}

void OpenGLRenderer::drawRoundedRectBorder(const ABrush& brush,
                                           glm::vec2 position,
                                           glm::vec2 size,
                                           float radius,
                                           int borderWidth) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(*mRoundedSolidShaderBorder),
            CustomShaderHelper{},
    }, brush);

    identityUv();
    glm::vec2 innerSize = { size.x - borderWidth * 2,
                            size.y - borderWidth * 2 };

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    gl::Program::currentShader()->set(aui::ShaderUniforms::INNER_SIZE, 2.f * (radius - borderWidth) / innerSize);
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, size / innerSize);
    uploadToShaderCommon();
    drawRectImpl(position, size);
}

void OpenGLRenderer::drawBoxShadow(glm::vec2 position,
                                   glm::vec2 size,
                                   float blurRadius,
                                   const AColor& color) {
    AUI_ASSERTX(blurRadius >= 0.f, "blurRadius is expected to be non negative, use drawBoxShadowInner for inset shadows instead");
    identityUv();
    mBoxShadowShader->use();
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, blurRadius / 2.f);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, position + size);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, position);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_TRANSFORM, mTransform);
    mBoxShadowShader->set(aui::ShaderUniforms::COLOR, mColor * color);

    mRectangleVao.bind();

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
    mRectangleVao.insert(0, AArrayView(uvs), "drawBoxShadow");

    mRectangleVao.drawElements();
}

void OpenGLRenderer::drawBoxShadowInner(glm::vec2 position,
                                        glm::vec2 size,
                                        float blurRadius,
                                        float spreadRadius,
                                        float borderRadius,
                                        const AColor& color,
                                        glm::vec2 offset) {
    AUI_ASSERTX(blurRadius >= 0.f, "blurRadius is expected to be non negative");
    blurRadius *= -1.f;
    identityUv();
    mBoxShadowInnerShader->use();
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, blurRadius / 2.f);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, position + offset + size - spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, position + offset + spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_TRANSFORM, mTransform);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::COLOR, mColor * color);
    
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * borderRadius / size);

    mRectangleVao.bind();

    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    const glm::vec2 uvs[] = {
        { x, h },
        { w, h },
        { x, y },
        { w, y },
    };
    mRectangleVao.insert(0, AArrayView(uvs), "drawBoxShadowInner");

    mRectangleVao.drawElements();
}
void OpenGLRenderer::drawString(glm::vec2 position,
                                const AString& string,
                                const AFontStyle& fs) {
    prerenderString(position, string, fs)->draw();
}

void OpenGLRenderer::setBlending(Blending blending) {
    switch (blending) {
        case Blending::NORMAL:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case Blending::INVERSE_DST:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            break;

        case Blending::ADDITIVE:
            glBlendFunc(GL_ONE, GL_ONE);
            break;

        case Blending::INVERSE_SRC:
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
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
    AOptional<gl::Vao> mVao;
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
    {
        if (mRenderer->isVaoAvailable()) {
            mVao.emplace();
            mVao->bind();
            mVertexBuffer.bind();
            mIndexBuffer.bind();
            setupVertexAttribs();
            gl::Vao::unbind();
        }
    }


    void draw() override {
        if (mIndexBuffer.count() == 0) return;

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

        if (mVao) {
            mVao->bind();
        } else {
            mVertexBuffer.bind();
            mIndexBuffer.bind();
            setupVertexAttribs();
        }

        auto finalColor = ARender::getColor() * mColor;
        if (mFontRendering == FontRendering::SUBPIXEL) {
            mRenderer->mSymbolShaderSubPixel->use();
            mRenderer->mSymbolShaderSubPixel->set(aui::ShaderUniforms::UV_SCALE, uvScale);
            mRenderer->mSymbolShaderSubPixel->set(aui::ShaderUniforms::TRANSFORM, mRenderer->getTransform());
            mRenderer->mSymbolShaderSubPixel->set(aui::ShaderUniforms::COLOR, glm::vec4(1, 1, 1, finalColor.a));
            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
            mIndexBuffer.drawWithoutBind(GL_TRIANGLES);

            mRenderer->mSymbolShaderSubPixel->set(aui::ShaderUniforms::COLOR, finalColor);
            glBlendFunc(GL_ONE, GL_ONE);
            mIndexBuffer.drawWithoutBind(GL_TRIANGLES);

            // reset blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            mRenderer->mSymbolShader->use();
            mRenderer->mSymbolShader->set(aui::ShaderUniforms::UV_SCALE, uvScale);
            mRenderer->mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, mRenderer->getTransform());
            mRenderer->mSymbolShader->set(aui::ShaderUniforms::COLOR, finalColor);
            mIndexBuffer.drawWithoutBind(GL_TRIANGLES);
        }
    }

    int getWidth() override {
        return mTextWidth;
    }

    int getHeight() override {
        return mTextHeight;
    }

private:
    static void setupVertexAttribs() {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex), reinterpret_cast<const void*>(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex), reinterpret_cast<const void*>(sizeof(glm::vec2)));
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
        gl::Vao::unbind();
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

bool OpenGLRenderer::isVaoAvailable() const noexcept {
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

bool OpenGLRenderer::setupLineShader(const ABrush& brush, const ABorderStyle& style, float widthPx) {
    return std::visit(aui::lambda_overloaded {
        [&](const ABorderStyle::Solid&) {
            std::visit(aui::lambda_overloaded {
                    GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
                    TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
                    SolidShaderHelper(*mSolidShader),
                    CustomShaderHelper{},
            }, brush);

            return false;
        },
        [&](const ABorderStyle::Dashed& dashed) {
            std::visit(aui::lambda_overloaded {
                    GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
                    TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
                    SolidShaderHelper(*mLineSolidDashedShader),
                    CustomShaderHelper{},
            }, brush);
            float dashWidth = dashed.dashWidth.valueOr(1.f) * widthPx; 
            float sumOfLengths = dashWidth + dashed.spaceBetweenDashes.valueOr(2.f) * widthPx;
            dashWidth *= APlatform::getDpiRatio();
            sumOfLengths *= APlatform::getDpiRatio();
            gl::Program::currentShader()->set(aui::ShaderUniforms::DIVIDER, sumOfLengths);
            gl::Program::currentShader()->set(aui::ShaderUniforms::THRESHOLD, dashWidth);

            return true;
        }
    }, style.value());
}

namespace {
    struct LineVertex {
        glm::vec2 position;
        float distance;
        float unused;
    };
}

void OpenGLRenderer::drawLines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    if (points.size() < 2) return;
    const auto widthPx = width.getValuePx();
    bool computeDistances = setupLineShader(brush, style, widthPx);
    uploadToShaderCommon();
    glLineWidth(widthPx);

    mRectangleVao.bind();


    AVector<LineVertex> positions;
    positions.reserve(points.size());
    positions << LineVertex{ points[0], 0.f, 1.f };

    float distanceAccumulator = 0.f;
    for (const auto& point : points | ranges::views::drop(1)) {
        if (computeDistances) {
            distanceAccumulator += glm::distance(positions.last().position, point);
        }
        positions << LineVertex { point, distanceAccumulator, 1.f };
    }

    mRectangleVao.insert(0, AArrayView(positions), "drawLines");
    mRectangleVao.drawArrays(GL_LINE_STRIP, points.size());
}

void OpenGLRenderer::drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) {
    const auto widthPx = width.getValuePx();
    bool computeDistances = setupLineShader(brush, style, widthPx);
    uploadToShaderCommon();
    glLineWidth(widthPx);

    mRectangleVao.bind();

    AVector<LineVertex> positions;
    positions.reserve(points.size() * 2);

    for (const auto& [p1, p2] : points) {
        positions << LineVertex {
            p1,
            0.f,
            1.f
        };
        positions << LineVertex {
            p2,
            glm::distance(p1, p2),
            1.f
        };
    }

    mRectangleVao.insert(0, AArrayView(positions), "drawLines");
    mRectangleVao.drawArrays(GL_LINES, positions.size());
}

void OpenGLRenderer::drawPoints(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    if (points.size() == 0) {
        return;
    }

    std::visit(aui::lambda_overloaded {
            GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
            TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
            SolidShaderHelper(*mSolidShader),
            CustomShaderHelper{},
    }, brush);

    const auto widthPx = size.getValuePx();
    uploadToShaderCommon();


#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
    // TODO slow, use instancing instead
    for (auto point : points) {
        drawRectImpl(point - glm::vec2(widthPx / 2), glm::vec2(widthPx));
    }
#else
    glPointSize(widthPx);

    mRectangleVao.bind();
    mRectangleVao.insert(0, AArrayView(points), "drawPoints");
    mRectangleVao.drawArrays(GL_POINTS, points.size());
#endif
}

void OpenGLRenderer::drawSquareSector(const ABrush& brush,
                                      const glm::vec2& position,
                                      const glm::vec2& size,
                                      AAngleRadians begin,
                                      AAngleRadians end) {
    std::visit(aui::lambda_overloaded {
            UnsupportedBrushHelper<ALinearGradientBrush>(),
            UnsupportedBrushHelper<ATexturedBrush>(),
            SolidShaderHelper(*mSquareSectorShader),
            CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();


    auto calculateLineMatrix = [](AAngleRadians angle) {
        auto s = glm::sin(angle.radians());
        auto c = glm::cos(angle.radians());
        return glm::mat3{c, 0, 0,
                         s, 0, 0,
                         -0.5f * (s + c), 0, 0};
    };
    auto m1 = calculateLineMatrix(begin + 180_deg);
    auto m2 = calculateLineMatrix(end);
    float whichAlgo = (end - begin).radians() >= glm::pi<float>();
    gl::Program::currentShader()->set(aui::ShaderUniforms::WHICH_ALGO, whichAlgo);
    gl::Program::currentShader()->set(aui::ShaderUniforms::M1, m1);
    gl::Program::currentShader()->set(aui::ShaderUniforms::M2, m2);

    drawRectImpl(position, size);
}


void OpenGLRenderer::beginPaint(glm::uvec2 windowSize) {
    gl::State::activeTexture(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    gl::State::bindVertexArray(0);
    gl::State::useProgram(0);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(1.f, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // stencil
    glStencilMask(0xff);
    glClearStencil(0);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xff);
}

void OpenGLRenderer::endPaint() {

}


void OpenGLRenderer::bindTemporaryVao() const noexcept {
    if (!isVaoAvailable()) {
        return;
    }
    mRectangleVao.bind();
}
