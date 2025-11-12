/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
#include "AUI/Render/IRenderer.h"
#include "glm/fwd.hpp"
#include "AUI/Util/GaussianKernel.h"
#include "AUI/Traits/bit.h"
#include <AUI/Traits/callables.h>
#include <AUI/Platform/AFontManager.h>
#include <AUI/GL/Vbo.h>
#include <AUI/GL/State.h>
#include <AUI/GL/RenderTarget/RenderbufferRenderTarget.h>
#include <AUI/Platform/ASurface.h>
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
#include <AUISL/Generated/rect_unblend.fsh.glsl120.h>
#include <AUISL/Generated/border_rounded.fsh.glsl120.h>
#include <AUISL/Generated/symbol.vsh.glsl120.h>
#include <AUISL/Generated/symbol.fsh.glsl120.h>
#include <AUISL/Generated/symbol_sub.fsh.glsl120.h>
#include <AUISL/Generated/line_solid_dashed.fsh.glsl120.h>
#include <AUISL/Generated/square_sector.fsh.glsl120.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/GL/RenderTarget/TextureRenderTarget.h>
#include <range/v3/algorithm/min_element.hpp>

static constexpr auto LOG_TAG = "OpenGLRenderer";

class OpenGLTexture2D : public ITexture {
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

    GradientShaderHelper(OpenGLRenderer& renderer, gl::Program& shader, gl::Texture2D& tex) : renderer(renderer),
                                                                                              shader(shader),
                                                                                              tex(tex) {}

    void operator()(const ALinearGradientBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, renderer.getColor());
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
    OpenGLRenderer& renderer;
    gl::Program& shader;

    SolidShaderHelper(OpenGLRenderer& renderer, gl::Program& shader) : renderer(renderer), shader(shader) {}

    void operator()(const ASolidBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, renderer.getColor() * brush.solidColor);
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

    TexturedShaderHelper(OpenGLRenderer& renderer, gl::Program& shader, gl::Vao& tempVao) : renderer(renderer),
                                                                                            shader(shader),
                                                                                            tempVao(tempVao) {}

    void operator()(const ATexturedBrush& brush) const {
        shader.use();
        shader.set(aui::ShaderUniforms::COLOR, renderer.getColor());
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
        tex->texture().setupRepeat();
        switch (brush.repeat) {
            case Repeat::NONE:
                tex->texture().setupClampToEdge();
                break;
            case Repeat::X_Y:
                tex->texture().setupRepeat();
                break;
            case Repeat::X:
            case Repeat::Y: {
                AUI_ASSERT_NO_CONDITION("Repeat::X and Repeat::Y are deprecated");
            }
        }
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
    out->loadBoth(Vertex::code(), Fragment::code());
    Vertex::setup(out->handle());
    Fragment::setup(out->handle());
    out->compile();
}
}

bool OpenGLRenderer::loadGL(GLLoadProc load_proc, bool es) {
    return es ? gladLoadGLES2Loader((GLADloadproc)load_proc) :
                gladLoadGLLoader((GLADloadproc)load_proc);
}

bool OpenGLRenderer::loadGL(GLLoadProc load_proc) {
    auto* get_string_proc = reinterpret_cast<PFNGLGETSTRINGPROC>(load_proc("glGetString"));
    if (get_string_proc == nullptr) return false;
    const GLubyte* ret_string = get_string_proc(GL_VERSION);
    if (!ret_string) return false;
    std::string_view version(reinterpret_cast<const char*>(ret_string));
    bool is_es = version.find("OpenGL ES") != std::string_view::npos;
    return loadGL(load_proc, is_es);
}

OpenGLRenderer::OpenGLRenderer() {
    ALogger::info(LOG_TAG) << "GL_VERSION = " << ((const char*) glGetString(GL_VERSION));
    ALogger::info(LOG_TAG) << "GL_VENDOR = " << ((const char*) glGetString(GL_VENDOR));
    ALogger::info(LOG_TAG) << "GL_RENDERER = " << ((const char*) glGetString(GL_RENDERER));
    ALogger::info(LOG_TAG) << "GL_EXTENSIONS = " << [] {
      if (auto ext = ((const char*) glGetString(GL_EXTENSIONS))) {
          return ext;
      } else {
          return "null";
      }
    }();
#if AUI_DEBUG && (AUI_PLATFORM_WIN || AUI_PLATFORM_LINUX)
    gl::setupDebug();
#endif

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
        aui::sl_gen::rect_unblend::fsh::glsl120::Shader>(mUnblendShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
        aui::sl_gen::square_sector::fsh::glsl120::Shader>(mSquareSectorShader);

    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader,
        aui::sl_gen::symbol::fsh::glsl120::Shader>(mSymbolShader);
    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader,
        aui::sl_gen::symbol_sub::fsh::glsl120::Shader>(mSymbolShaderSubPixel);

    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader,
        aui::sl_gen::line_solid_dashed::fsh::glsl120::Shader>(mLineSolidDashedShader);

    {
        constexpr GLuint INDICES[] = {0, 1, 2, 2, 1, 3};
        mRectangleVao.indices(INDICES);
    }
    {
        constexpr GLuint INDICES[] = {0, 1, 2, 3, 4, 5, 6, 7};
        mBorderVao.indices(INDICES);
    }
}

glm::mat4 OpenGLRenderer::getProjectionMatrix() const {
    return glm::ortho(0.0f, static_cast<float>(mWindow->getWidth()),
                      static_cast<float>(mWindow->getHeight()), 0.0f,
                      -1.f, 1.f);
}

void OpenGLRenderer::uploadToShaderCommon() {
    gl::Program::currentShader()->set(aui::ShaderUniforms::TRANSFORM, mTransform);
}

std::array<glm::vec2, 4> OpenGLRenderer::getVerticesForRect(glm::vec2 position, glm::vec2 size) {
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;

    return
        {
            glm::vec2{x, h,},
            glm::vec2{w, h,},
            glm::vec2{x, y,},
            glm::vec2{w, y,},
        };
}

void OpenGLRenderer::rectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size) {
    std::visit(aui::lambda_overloaded{
        GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
        TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
        SolidShaderHelper(*this, *mSolidShader),
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

void OpenGLRenderer::roundedRectangle(const ABrush& brush,
                                      glm::vec2 position,
                                      glm::vec2 size,
                                      float radius) {
    std::visit(aui::lambda_overloaded{
        GradientShaderHelper(*this, *mRoundedGradientShader, mGradientTexture),
        UnsupportedBrushHelper<ATexturedBrush>(),
        SolidShaderHelper(*this, *mRoundedSolidShader),
        CustomShaderHelper{},
    }, brush);
    uploadToShaderCommon();
    identityUv();

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    drawRectImpl(position, size);
}

void OpenGLRenderer::rectangleBorder(const ABrush& brush,
                                     glm::vec2 position,
                                     glm::vec2 size,
                                     float lineWidth) {
    std::visit(aui::lambda_overloaded{
        UnsupportedBrushHelper<ALinearGradientBrush>(),
        UnsupportedBrushHelper<ATexturedBrush>(),
        SolidShaderHelper(*this, *mSolidShader),
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
                             glm::vec3(glm::vec4{x + lineWidth, y + lineDelta, 1, 1}),
                             glm::vec3(glm::vec4{w, y + lineDelta, 1, 1}),

                             glm::vec3(glm::vec4{w - lineDelta, y + lineWidth, 1, 1}),
                             glm::vec3(glm::vec4{w - lineDelta, h, 1, 1}),

                             glm::vec3(glm::vec4{w - lineWidth, h - lineDelta - 0.15f, 1, 1}),
                             glm::vec3(glm::vec4{x, h - lineDelta - 0.15f, 1, 1}),

                             glm::vec3(glm::vec4{x + lineDelta, h - lineWidth - 0.15f, 1, 1}),
                             glm::vec3(glm::vec4{x + lineDelta, y, 1, 1}),
                         }), "rectangleBorder");

    glLineWidth(lineWidth);
    mRectangleVao.drawElements(GL_LINES);
}

void OpenGLRenderer::roundedRectangleBorder(const ABrush& brush,
                                            glm::vec2 position,
                                            glm::vec2 size,
                                            float radius,
                                            int borderWidth) {
    std::visit(aui::lambda_overloaded{
        UnsupportedBrushHelper<ALinearGradientBrush>(),
        UnsupportedBrushHelper<ATexturedBrush>(),
        SolidShaderHelper(*this, *mRoundedSolidShaderBorder),
        CustomShaderHelper{},
    }, brush);

    identityUv();
    glm::vec2 innerSize = {size.x - borderWidth * 2,
                           size.y - borderWidth * 2};

    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_SIZE, 2.f * radius / size);
    gl::Program::currentShader()->set(aui::ShaderUniforms::INNER_SIZE, 2.f * (radius - borderWidth) / innerSize);
    gl::Program::currentShader()->set(aui::ShaderUniforms::OUTER_TO_INNER, size / innerSize);
    uploadToShaderCommon();
    drawRectImpl(position, size);
}

void OpenGLRenderer::boxShadow(glm::vec2 position,
                               glm::vec2 size,
                               float blurRadius,
                               const AColor& color) {
    AUI_ASSERTX(blurRadius >= 0.f,
                "blurRadius is expected to be non negative, use boxShadowInner for inset shadows instead");
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
        {x, h},
        {w, h},
        {x, y},
        {w, y},
    };
    mRectangleVao.insert(0, AArrayView(uvs), "boxShadow");

    mRectangleVao.drawElements();
}

void OpenGLRenderer::boxShadowInner(glm::vec2 position,
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
        {x, h},
        {w, h},
        {x, y},
        {w, y},
    };
    mRectangleVao.insert(0, AArrayView(uvs), "boxShadowInner");

    mRectangleVao.drawElements();
}

void OpenGLRenderer::string(glm::vec2 position,
                            const AString& string,
                            const AFontStyle& fs) {
    prerenderString(position, string, fs)->draw();
}

void OpenGLRenderer::setBlending(Blending blending) {
    if (glBlendFuncSeparate) {
        switch (blending) {
            case Blending::NORMAL: {
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                return;
            }

            case Blending::INVERSE_DST:
                glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                return;

            case Blending::ADDITIVE:
                glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                return;

            case Blending::INVERSE_SRC:
                glBlendFuncSeparate(GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                return;
        }
    }
    //    glBlendEquation(GL_FUNC_ADD);
    switch (blending) {
        case Blending::NORMAL:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            return;

        case Blending::INVERSE_DST:
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
            return;

        case Blending::ADDITIVE:
            glBlendFunc(GL_ONE, GL_ONE);
            return;

        case Blending::INVERSE_SRC:
            glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
            return;
    }
}

class OpenGLPrerenderedString : public IRenderer::IPrerenderedString {
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
    FontRendering mFontRendering;

    OpenGLPrerenderedString(OpenGLRenderer* renderer,
                            gl::VertexBuffer vertexBuffer,
                            gl::IndexBuffer indexBuffer,
                            int textWidth,
                            int textHeight,
                            OpenGLRenderer::FontEntryData* entryData,
                            FontRendering fontRendering) :
        mRenderer(renderer),
        mVertexBuffer(std::move(vertexBuffer)),
        mIndexBuffer(std::move(indexBuffer)),
        mTextWidth(textWidth),
        mTextHeight(textHeight),
        mEntryData(entryData),
        mFontRendering(fontRendering) {
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

        if (AWindow::current()->profiling()->showBaseline) {
            mRenderer->rectangle(
                ASolidBrush { AColor::RED.transparentize(0.5f) }, { 0, 0 }, { mTextWidth, 1 });   // debug baseline
        }

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

        auto finalColor = mRenderer->getColor();
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
            mRenderer->setBlending(Blending::NORMAL);
        } else {
            mRenderer->setBlending(Blending::NORMAL);
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

        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex),
                              reinterpret_cast<const void*>(0));
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(OpenGLPrerenderedString::Vertex),
                              reinterpret_cast<const void*>(sizeof(glm::vec2)));
    }
};

class OpenGLMultiStringCanvas : public IRenderer::IMultiStringCanvas {
private:
    AVector<OpenGLPrerenderedString::Vertex> mVertices;
    OpenGLRenderer* mRenderer;
    AFontStyle mFontStyle;
    OpenGLRenderer::FontEntryData* mEntryData;
    int mAdvanceX = 0;
    int mAdvanceY = 0;

public:
    OpenGLMultiStringCanvas(OpenGLRenderer* renderer, const AFontStyle& fontStyle) :
        mRenderer(renderer),
        mFontStyle(fontStyle),
        mEntryData(renderer->getFontEntryData(fontStyle)) {
        mVertices.reserve(1000);
    }

    template<class UnicodeString>
    void addStringT(const glm::ivec2& position, UnicodeString text) noexcept {
        mVertices.reserve(mVertices.capacity() + text.length() * 4);
        auto& font = mFontStyle.font;
        auto& texturePacker = mEntryData->texturePacker;
        auto fe = mFontStyle.getFontEntry();

        const bool hasKerning = font->isHasKerning();

        int advanceX = position.x;
        int advanceY = position.y;
        size_t counter = 0;
        float advance = advanceX;
        for (auto i = text.begin(); i != text.end(); ++i, ++counter) {
            AChar c = *i;
            if (c == ' ') {
                notifySymbolAdded({glm::ivec2{advance, advanceY}});
                advance += mFontStyle.getSpaceWidth();
            } else if (c == '\n') {
                notifySymbolAdded({glm::ivec2{advance, advanceY}});
                advanceX = (glm::max)(advanceX, int(glm::ceil(advance)));
                advance = position.x;
                advanceY += mFontStyle.getLineHeight();
                nextLine();
            } else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    advance += mFontStyle.getSpaceWidth();
                    continue;
                }
                if ((advance >= 0 && advance <= 99999) /* || gui3d */) {

                    int posX = advance + ch.horizontal.bearing.x;
                    int posY = advanceY - ch.horizontal.bearing.y;
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

                    notifySymbolAdded({glm::ivec2{posX, posY}});
                    mVertices.push_back({glm::vec2(posX, posY + height),
                                         glm::vec2(uv.x, uv.w)});
                    mVertices.push_back({glm::vec2(posX + width, posY + height),
                                         glm::vec2(uv.z, uv.w)});
                    mVertices.push_back({glm::vec2(posX, posY),
                                         glm::vec2(uv.x, uv.y)});
                    mVertices.push_back({glm::vec2(posX + width, posY),
                                         glm::vec2(uv.z, uv.y)});

                }

                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != text.end()) {
                        auto kerning = font->getKerning(c, *next);
                        advance += kerning.x;
                    }
                }

                advance += ch.horizontal.advance;
            }
        }

        notifySymbolAdded({glm::ivec2{advance, advanceY}});

        mAdvanceX = (glm::max)(mAdvanceX, (glm::max)(advanceX, int(glm::ceil(advance))));
        mAdvanceY = advanceY + mFontStyle.getLineHeight();
    }

    void addString(const glm::ivec2& position, AStringView text) noexcept override {
        addStringT(position, text);
    }

    void addString(const glm::ivec2& position, std::u32string_view text) noexcept override {
        addStringT(position, text);
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
                                             mFontStyle.fontRendering);
    }

    ~OpenGLMultiStringCanvas() override = default;
};

_<IRenderer::IPrerenderedString> OpenGLRenderer::prerenderString(glm::vec2 position,
                                                                 const AString& text,
                                                                 const AFontStyle& fs) {
    //    ALOG_DEBUG("OpenGL") << "prerenderString: " << text;
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

_unique<ITexture> OpenGLRenderer::createNewTexture() {
    return std::make_unique<OpenGLTexture2D>();
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
    return std::visit(aui::lambda_overloaded{
        [&](const ABorderStyle::Solid&) {
          std::visit(aui::lambda_overloaded{
              GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
              TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
              SolidShaderHelper(*this, *mSolidShader),
              CustomShaderHelper{},
          }, brush);

          return false;
        },
        [&](const ABorderStyle::Dashed& dashed) {
          std::visit(aui::lambda_overloaded{
              GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
              TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
              SolidShaderHelper(*this, *mLineSolidDashedShader),
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

void
OpenGLRenderer::lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) {
    if (points.size() < 2) return;
    const auto widthPx = width.getValuePx();
    bool computeDistances = setupLineShader(brush, style, widthPx);
    uploadToShaderCommon();
    glLineWidth(widthPx);

    mRectangleVao.bind();


    AVector<LineVertex> positions;
    positions.reserve(points.size());
    positions << LineVertex{points[0], 0.f, 1.f};

    float distanceAccumulator = 0.f;
    for (const auto& point: points | ranges::views::drop(1)) {
        if (computeDistances) {
            distanceAccumulator += glm::distance(positions.last().position, point);
        }
        positions << LineVertex{point, distanceAccumulator, 1.f};
    }

    mRectangleVao.insert(0, AArrayView(positions), "lines");
    mRectangleVao.drawArrays(GL_LINE_STRIP, points.size());
}

void OpenGLRenderer::lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points,
                           const ABorderStyle& style, AMetric width) {
    const auto widthPx = width.getValuePx();
    bool computeDistances = setupLineShader(brush, style, widthPx);
    uploadToShaderCommon();
    glLineWidth(widthPx);

    mRectangleVao.bind();

    AVector<LineVertex> positions;
    positions.reserve(points.size() * 2);

    for (const auto& [p1, p2]: points) {
        positions << LineVertex{
            p1,
            0.f,
            1.f
        };
        positions << LineVertex{
            p2,
            glm::distance(p1, p2),
            1.f
        };
    }

    mRectangleVao.insert(0, AArrayView(positions), "lines");
    mRectangleVao.drawArrays(GL_LINES, positions.size());
}

void OpenGLRenderer::points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) {
    if (points.size() == 0) {
        return;
    }

    std::visit(aui::lambda_overloaded{
        GradientShaderHelper(*this, *mGradientShader, mGradientTexture),
        TexturedShaderHelper(*this, *mTexturedShader, mRectangleVao),
        SolidShaderHelper(*this, *mSolidShader),
        CustomShaderHelper{},
    }, brush);

    const auto widthPx = size.getValuePx();
    uploadToShaderCommon();


#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS || AUI_PLATFORM_EMSCRIPTEN
    // TODO slow, use instancing instead
   for (auto point : points) {
       drawRectImpl(point - glm::vec2(widthPx / 2), glm::vec2(widthPx));
   }
#else
    glPointSize(widthPx);

    mRectangleVao.bind();
    mRectangleVao.insert(0, AArrayView(points), "points");
    mRectangleVao.drawArrays(GL_POINTS, points.size());
#endif
}

void OpenGLRenderer::squareSector(const ABrush& brush,
                                  const glm::vec2& position,
                                  const glm::vec2& size,
                                  AAngleRadians begin,
                                  AAngleRadians end) {
    std::visit(aui::lambda_overloaded{
        UnsupportedBrushHelper<ALinearGradientBrush>(),
        UnsupportedBrushHelper<ATexturedBrush>(),
        SolidShaderHelper(*this, *mSquareSectorShader),
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


static void resetStencil() {
    glStencilMask(0xff);
    glClearStencil(0);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xff);
}

void OpenGLRenderer::beginPaint(glm::uvec2 windowSize) {
    gl::State::activeTexture(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    gl::State::bindVertexArray(0);
    gl::State::useProgram(0);


    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    setBlending(Blending::NORMAL);

    resetStencil();
}

void OpenGLRenderer::endPaint() {
    gl::State::activeTexture(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    gl::State::bindVertexArray(0);
    gl::State::useProgram(0);
}


void OpenGLRenderer::bindTemporaryVao() const noexcept {
    if (!isVaoAvailable()) {
        return;
    }
    mRectangleVao.bind();
}

_unique<IRenderViewToTexture> OpenGLRenderer::newRenderViewToTexture() noexcept {
#if !AUI_PLATFORM_EMSCRIPTEN
    AUI_ASSERT(allowRenderToTexture());
    if (!glBlendFuncSeparate) {
        return nullptr;
    }
    try {
        class OpenGLRenderViewToTexture : public IRenderViewToTexture {
        public:
            explicit OpenGLRenderViewToTexture(OpenGLRenderer& renderer) : mRenderer(renderer) {
                auto prevFramebuffer = gl::Framebuffer::current();
                AUI_DEFER { AUI_NULLSAFE(prevFramebuffer)->bind(); else gl::Framebuffer::unbind(); };

                mFramebuffer.setSupersamplingRatio(1);
                mFramebuffer.resize({1, 1});
                auto albedo = _new<gl::TextureRenderTarget<gl::InternalFormat::RGBA8, gl::Type::UNSIGNED_BYTE, gl::Format::RGBA>>();
                mFramebuffer.attach(albedo, GL_COLOR_ATTACHMENT0);
                albedo->texture().setupClampToEdge();
                albedo->texture().setupNearest();
                mTexture = std::move(albedo);
            }

            static gl::Framebuffer* getMainRenderingFramebuffer(IRenderer& renderer) {
                return dynamic_cast<OpenGLRenderingContext*>(renderer.getWindow()->getRenderingContext().get())->framebuffer().valueOr(nullptr);
            }

            bool begin(IRenderer& renderer, glm::ivec2 surfaceSize, IRenderViewToTexture::InvalidArea& invalidArea) override {
                AUI_ASSERT(!invalidArea.empty()); // if invalid areas are empty, what should we redraw then?
                AUI_ASSERT(&mRenderer == &renderer);
                auto mainRenderingFB = getMainRenderingFramebuffer(renderer);
                AUI_ASSERT(mainRenderingFB != nullptr);
                if (glm::any(glm::greaterThan(glm::u32vec2(surfaceSize), mainRenderingFB->size()))) {
                    return false;
                }
                mRenderer.setTransformForced(mRenderer.getProjectionMatrix());
                AUI_ASSERT(mRenderer.mRenderToTextureTarget == nullptr);
                mRenderer.mRenderToTextureTarget = this;
                mRenderer.setBlending(Blending::NORMAL);
                mRenderer.setStencilDepth(0);

                AUI_DEFER { mainRenderingFB->bind(); };

                {
                    auto prevSize = mFramebuffer.size();
                    mFramebuffer.resize(surfaceSize);
                    if (prevSize != mFramebuffer.size()) [[unlikely]] {
                        // switching to full redraw - we have lost previous data.
                        invalidArea = InvalidArea::Full{};
                        return true; // omit partial update checks
                    }
                }

                if (auto rectangles = invalidArea.rectangles()) {
                    // partial update.
                    // 1. copy "cached" pixel data from our framebuffer to main rendering fb
                    {
                        mFramebuffer.bindForRead();
                        mainRenderingFB->bindForWrite();
                        const auto supersampledRenderingFBSize =
                            mFramebuffer.size() * mainRenderingFB->supersamlingRatio();
                        glBlitFramebuffer(0, 0, mFramebuffer.size().x, mFramebuffer.size().y,
                                          0, mainRenderingFB->supersampledSize().y - supersampledRenderingFBSize.y, supersampledRenderingFBSize.x, mainRenderingFB->supersampledSize().y,
                                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
                    }

                    // 2. mark invalidated rectangles with stencil mask.
                    glStencilFunc(GL_ALWAYS, 1, 0xff);
                    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
                    glStencilMask(0xff);
                    // also temporarily disable GL_BLEND; we'll draw rects of color (0, 0, 0, 0) effectively resetting
                    // pixels.
                    glDisable(GL_BLEND);
                    AUI_DEFER {
                      glEnable(GL_BLEND);
                      glStencilMask(0x00);
                      glStencilFunc(GL_EQUAL, ++mRenderer.mStencilDepth, 0xff);
                    };
                    static constexpr auto MAX_RECTANGLE_COUNT = std::decay_t<decltype(*rectangles)>::capacity();
                    using Vertices = AStaticVector<glm::vec2, MAX_RECTANGLE_COUNT * 4>;
                    using Indices = AStaticVector<GLuint, MAX_RECTANGLE_COUNT * 6>;
                    auto vertices =  ranges::views::transform(*rectangles, [&](const ARect<int>& r) {
                      return getVerticesForRect(r.p1, r.size());
                    }) | ranges::views::join | ranges::to<Vertices>();
                    mRenderer.mRectangleVao.insert(0, AArrayView(vertices), "render-to-texture invalid areas");
                    auto indices = ranges::views::generate([i = 0]() mutable {
                      int offset = 4 * i++;
                      return std::array<GLuint, 6>{GLuint(offset + 0),
                                                   GLuint(offset + 1),
                                                   GLuint(offset + 2),
                                                   GLuint(offset + 2),
                                                   GLuint(offset + 1),
                                                   GLuint(offset + 3)};
                    }) | ranges::views::take_exactly(rectangles->size())
                                   | ranges::views::join
                                   | ranges::to<Indices>()
                    ;
                    static constexpr auto DEFAULT_INDICES_SIZE = 6;
                    if (indices.size() != DEFAULT_INDICES_SIZE) mRenderer.mRectangleVao.indices(AArrayView(indices));
                    AUI_DEFER {
                      if (indices.size() != DEFAULT_INDICES_SIZE) {
                          mRenderer.mRectangleVao.indices(AArrayView(indices.data(), 6));
                      }
                    };
                    mRenderer.mSolidShader->use();
                    mRenderer.uploadToShaderCommon();
                    mRenderer.mSolidShader->set(aui::ShaderUniforms::COLOR, glm::vec4(0.f));
                    mRenderer.mRectangleVao.drawElements();
                }

                return true;
            }

            void end(IRenderer& renderer) override {
                AUI_ASSERT(&mRenderer == &renderer);
                AUI_ASSERT(mRenderer.mRenderToTextureTarget == this);
                mRenderer.mRenderToTextureTarget = nullptr;
                auto mainRenderingFB = gl::Framebuffer::current();
                AUI_ASSERT(mainRenderingFB != nullptr);
                AUI_DEFER {
                  mainRenderingFB->bind();
                  glStencilMask(0xff);
                  glClearColor(0, 0, 0, 0);
                  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                  glStencilFunc(GL_EQUAL, 0x00, 0xff);
                  glStencilMask(0);
                  mRenderer.setStencilDepth(0);
                };

                // copy render results from mainRenderingFB (main render buffer) to our texture render target.
                // GL_LINEAR resolves supersampling.
                mainRenderingFB->bindForRead();
                mFramebuffer.bindForWrite();
                const auto supersampledRenderingFBSize = mFramebuffer.size() * mainRenderingFB->supersamlingRatio();
                glBlitFramebuffer(0, mainRenderingFB->supersampledSize().y - supersampledRenderingFBSize.y, supersampledRenderingFBSize.x, mainRenderingFB->supersampledSize().y,
                                  0, 0, mFramebuffer.size().x, mFramebuffer.size().y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            }

            void draw(IRenderer& renderer) override {
                AUI_ASSERT(&mRenderer == &renderer);
                mRenderer.mUnblendShader->use();
                mRenderer.mUnblendShader->set(aui::ShaderUniforms::COLOR, renderer.getColor());
                mTexture->bindAsTexture(0);
                mRenderer.identityUv();
                mRenderer.uploadToShaderCommon();

                const glm::vec2 uvs[] = {
                    {0, 0},
                    {1, 0},
                    {0, 1},
                    {1, 1}
                };
                mRenderer.mRectangleVao.insertIfKeyMismatches(1, AArrayView(uvs), "OpenGLRenderViewToTexture");
                mRenderer.drawRectImpl({0, 0}, mFramebuffer.size());
                if (auto& p = AWindow::current()->profiling(); p && p->renderToTextureDecay) [[unlikely]] {
                    // decays to fast. attach it to time
                    using namespace std::chrono;
                    using namespace std::chrono_literals;
                    static auto lastDecayUpdate = high_resolution_clock::now();
                    if (auto now = high_resolution_clock::now(); now - lastDecayUpdate < 50ms) {
                        return;
                    } else {
                        lastDecayUpdate = now;
                    }

                    // bind our framebuffer and draw transparent blend which decreases pixel value by 1.
                    auto mainRenderingFB = gl::Framebuffer::current();
                    AUI_ASSERT(mainRenderingFB != nullptr);
                    AUI_DEFER {
                      // restore.
                      glColorMask(true, true, true, true);
                      mainRenderingFB->bind();
                      glEnable(GL_STENCIL_TEST);
                      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                      renderer.setStencilDepth(0);
                    };
                    mFramebuffer.bind();
                    glDisable(GL_STENCIL_TEST);
                    glColorMask(true, true, true, false);
                    auto& shader = mRenderer.mSolidShader;
                    shader->use();
                    shader->set(aui::ShaderUniforms::TRANSFORM, glm::mat4(1.f));
                    shader->set(aui::ShaderUniforms::COLOR, glm::vec4(0.5, 0.5, 0.5, 0.1));
                    mRenderer.drawRectImpl({-1, -1}, {2, 2}); // offscreen
                }
            }

        private:
            OpenGLRenderer& mRenderer;
            gl::Framebuffer mFramebuffer;
            _<gl::TextureRenderTarget<gl::InternalFormat::RGBA8, gl::Type::UNSIGNED_BYTE, gl::Format::RGBA>> mTexture;
        };
        return std::make_unique<OpenGLRenderViewToTexture>(*this);
    } catch (const AException& e) {
        ALogger::warn(LOG_TAG) << "Failed to initialize newRenderViewToTexture: " << e;
    }
#endif
    return nullptr;
}

void OpenGLRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops) {
    if (!glm::all(glm::greaterThan(size, glm::ivec2(0)))) {
        return;
    }

    if (backdrops.empty()) {
        return;
    }
    auto source = gl::Framebuffer::current();
    if (!source) {
        IRenderer::backdrops(position, size, backdrops);
        return;
    }

    struct AreaOfInterest {
        FramebufferFromPool framebuffer;
        glm::ivec2 size {};
    };

    AOptional<AreaOfInterest> areaOfInterest;

    auto initAreaOfInterestIfEmpty = [&](glm::ivec2 fbSize){
      if (areaOfInterest) {
          return;
      }

      auto fb = getFramebufferForMultiPassEffect(fbSize);
      AUI_ASSERT(glm::all(glm::lessThanEqual(glm::u32vec2(fbSize), fb->framebuffer.size())));
      source->bindForRead();
      fb->framebuffer.bindForWrite();

      auto vertices = getVerticesForRect(position, size);
      std::array<glm::ivec2, vertices.size()> transformedVertices{};
      for (size_t i = 0; i < vertices.size(); ++i) {
          auto ndc = glm::vec2(mTransform * glm::vec4(vertices[i], 0, 1));
          transformedVertices[i] = glm::ivec2((ndc + 1.f) / 2.f * glm::vec2(source->supersampledSize()));
      }

      auto p1 = transformedVertices.front();
      auto p2 = transformedVertices.back();
      glBlitFramebuffer(p1.x, p1.y, p2.x, p2.y, 0, 0, fbSize.x, fbSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
      areaOfInterest = AreaOfInterest{
          .framebuffer = std::move(fb),
          .size = fbSize,
      };
    };

    {
        glDisable(GL_STENCIL_TEST);
        AUI_DEFER {
          source->bind();
          source->bindViewport();
          glEnable(GL_STENCIL_TEST);
        };

        for (const auto& backdrop : backdrops) {
            RenderHints::PushState s(*this);
            std::visit(
                aui::lambda_overloaded {
                    [&](const ass::Backdrop::LiquidFluid& liquidFluid) {
                      if (areaOfInterest) {
                          throw AException("LiquidGlass must be the first effect in backdrop list.");
                      }
                      areaOfInterest = [&] {
                          auto targetSize = source->size();
                          auto fb = getFramebufferForMultiPassEffect(targetSize);
                          source->bindForRead();
                          fb->framebuffer.bindForWrite();
                          glBlitFramebuffer(
                              0, 0,                                                         // src
                              source->supersampledSize().x, source->supersampledSize().y,   //
                              0, 0,                                                         // dst
                              targetSize.x, targetSize.y,                                   //
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
                          return AreaOfInterest {
                              .framebuffer = std::move(fb),
                              .size = targetSize,
                          };
                      }();

                      areaOfInterest->framebuffer->rendertarget->texture().bind();
                      static auto uvMap = [] {
                          auto result = std::make_unique<gl::Texture2D>();
                          result->tex2D(*AImage::fromUrl(":uni/liquid_glass1.png"));
                          return result;
                      }();
                      uvMap->bind(1);
                      areaOfInterest->framebuffer->rendertarget->texture().setupNearest();
//                      areaOfInterest->framebuffer->rendertarget->texture().setupMirroredRepeat();
                      areaOfInterest->framebuffer->rendertarget->texture().setupClampToEdge();

                      auto offscreen1 = getFramebufferForMultiPassEffect(size);
                      offscreen1->framebuffer.bind();
                      offscreen1->framebuffer.bindViewport();

                      static auto shader = [&] {
                        auto shader = std::make_unique<gl::Program>();

                        shader->loadVertexShader(
                            std::string(aui::sl_gen::basic_uv::vsh::glsl120::Shader::code()));
                        shader->loadFragmentShader(
                            R"(
precision highp float;
precision highp int;
varying vec4 SL_inter_vertex;
varying vec2 SL_inter_uv;
uniform mat4 SL_uniform_m2;
uniform vec2 SL_uniform_uvScale;
uniform sampler2D SL_uniform_albedo;
uniform sampler2D uvmap;
void main() {
 vec4 uvmap_sample = texture2D(uvmap, SL_inter_uv.xy);
 vec2 base_uv = (vec2(SL_uniform_m2 * vec4(mix(vec2(-1.0), vec2(2.0), uvmap_sample.xy), 0.0, 1.0)) + vec2(1.0)) * vec2(0.5);
 base_uv = clamp(base_uv, vec2(0.0), vec2(1.0));
 base_uv = base_uv * SL_uniform_uvScale;
 vec3 accumulator = texture2D(SL_uniform_albedo, base_uv).xyz;
 gl_FragColor = vec4(accumulator.xyz, uvmap_sample.a);
}
)");

                        aui::sl_gen::basic_uv::vsh::glsl120::Shader::setup(shader->handle());
                        shader->compile();
                        shader->use();
                        shader->set(gl::Program::Uniform("uvmap"), 1);

                        return shader;
                      }();

                      if (!shader) {
                          // can't compile shader; actually, it would result in blur like behaviour
                          return;
                      }

                      shader->use();
                      {
                          auto m = mTransform;
                          m = glm::translate(m, glm::vec3(0, size.y, 0));
                          m = glm::scale(m, glm::vec3(1, -1, 1));
                          m = glm::translate(m, glm::vec3(position, 0.f));
                          m = glm::scale(m, glm::vec3(size, 1.f));
                          shader->set(aui::ShaderUniforms::M2, m);

                          // source framebuffer is slightly smaller than the framebuffer in area of interest, adjust it
                          shader->set(aui::ShaderUniforms::UV_SCALE, glm::vec2(areaOfInterest->size) / glm::vec2(areaOfInterest->framebuffer->framebuffer.size()));
                      }

                      setTransformForced(glm::ortho(
                          0.0f, static_cast<float>(offscreen1->framebuffer.size().x) - 0.0f,
                          0.f, static_cast<float>(offscreen1->framebuffer.size().y) - 0.0f, -1.f, 1.f));
                      uploadToShaderCommon();
                      identityUv();

                      glDisable(GL_BLEND);
                      drawRectImpl({}, size);
                      glEnable(GL_BLEND);

                      areaOfInterest = AreaOfInterest {
                          .framebuffer = std::move(offscreen1),
                          .size = size,
                      };
                    },
                    [&](const ass::Backdrop::GaussianBlurCustom& gaussianBlur) {
                      const auto radius = int(gaussianBlur.radius.getValuePx());
                      if (radius <= 1) {
                          return;
                      }
                      AUI_ASSERT(gaussianBlur.downscale > 0);

                      auto sizeDownscaled = size / gaussianBlur.downscale;

                      initAreaOfInterestIfEmpty(sizeDownscaled);
                      areaOfInterest->framebuffer->rendertarget->texture().bind();
                      areaOfInterest->framebuffer->rendertarget->texture().setupNearest();
                      areaOfInterest->framebuffer->rendertarget->texture().setupClampToEdge();

                      auto offscreen1 = getFramebufferForMultiPassEffect(sizeDownscaled);
                      offscreen1->framebuffer.bind();
                      offscreen1->framebuffer.bindViewport();

                      setTransformForced(glm::ortho(
                          0.0f, static_cast<float>(offscreen1->framebuffer.size().x) - 0.0f,
                          0.f, static_cast<float>(offscreen1->framebuffer.size().y) - 0.0f, -1.f, 1.f));

                      struct BlurShaders {
                          std::unique_ptr<gl::Program> shader;
                      };
                      static AUnorderedMap<unsigned /* radius */, BlurShaders> blurShaders;
                      auto& blurShader = blurShaders.getOrInsert(radius, [&] {
                        BlurShaders result {
                            .shader = std::make_unique<gl::Program>(),
                        };

                        auto kernel = aui::detail::gaussianKernel(radius);

                        result.shader->loadVertexShader(
                            std::string(aui::sl_gen::basic_uv::vsh::glsl120::Shader::code()));
                        result.shader->loadFragmentShader(
                            fmt::format(
                                R"(
precision highp float;
precision highp int;
varying vec2 SL_inter_uv;
uniform vec2 SL_uniform_m2;
uniform vec2 SL_uniform_pixel_to_uv;
uniform sampler2D SL_uniform_albedo;
uniform float SL_uniform_kernel[{kernel_size}];
void main() {{
 vec3 accumulator = vec3(0.0, 0.0, 0.0);
 vec2 base_uv = SL_uniform_m2 * SL_inter_uv;
 for (int i = -{radius}; i <= {radius}; ++i) {{
   float f = float(i);
   vec2 uv = SL_uniform_pixel_to_uv * vec2(f, f);
   uv += base_uv;
   uv = clamp(uv, vec2(0.0), SL_uniform_m2);
   accumulator += texture2D(SL_uniform_albedo, uv).xyz * SL_uniform_kernel[{radius} + i];
 }}
 gl_FragColor = vec4(accumulator, 1.0);
}}
)",
                                fmt::arg("radius", radius),
                                fmt::arg("kernel_size", kernel.size())));

                        aui::sl_gen::basic_uv::vsh::glsl120::Shader::setup(result.shader->handle());
                        result.shader->compile();
                        result.shader->use();
                        result.shader->setArray(aui::ShaderUniforms::KERNEL, kernel);

                        return result;
                      });

                      if (!blurShader.shader) {
                          // can't compile shader; actually, it would result in blur like behaviour
                          return;
                      }

                      blurShader.shader->use();
                      {
                          auto stepSize = glm::vec2(1.f / areaOfInterest->framebuffer->framebuffer.supersampledSize().x, 0.f);
                          blurShader.shader->set(aui::ShaderUniforms::PIXEL_TO_UV, stepSize);
                          blurShader.shader->set(
                              aui::ShaderUniforms::M2,
                              glm::vec2(areaOfInterest->size) /
                              glm::vec2(areaOfInterest->framebuffer->framebuffer.supersampledSize()) -
                              stepSize / 2.f /* small bias to avoid dirty edges */);
                      }
                      uploadToShaderCommon();
                      identityUv();
                      drawRectImpl({}, sizeDownscaled);

                      areaOfInterest.reset();   // release borrowed framebuffer before requesting another one

                      auto offscreen2 = getFramebufferForMultiPassEffect(sizeDownscaled);
                      offscreen1->rendertarget->texture().bind();
                      offscreen2->framebuffer.bind();
                      offscreen2->framebuffer.bindViewport();

                      setTransformForced(glm::ortho(
                          0.0f, static_cast<float>(offscreen2->framebuffer.size().x) - 0.0f,
                          0.f, static_cast<float>(offscreen2->framebuffer.size().y) - 0.0f, -1.f, 1.f));

                      blurShader.shader->set(aui::ShaderUniforms::M1, glm::vec2(0.0));

                      {
                          auto stepSize = glm::vec2(0.f, 1.f / float(offscreen1->framebuffer.supersampledSize().y));
                          blurShader.shader->set(aui::ShaderUniforms::PIXEL_TO_UV, stepSize);
                          blurShader.shader->set(
                              aui::ShaderUniforms::M2,
                              glm::vec2(sizeDownscaled) / glm::vec2(offscreen1->framebuffer.supersampledSize()) -
                              stepSize / 2.f);
                      }
                      uploadToShaderCommon();
                      drawRectImpl({}, sizeDownscaled);

                      areaOfInterest = AreaOfInterest {
                          .framebuffer = std::move(offscreen2),
                          .size = sizeDownscaled,
                      };
                    },
                },
                backdrop);
        }
    }

    auto& shader = *mTexturedShader;
    shader.use();
    shader.set(aui::ShaderUniforms::COLOR, getColor());
    {
        auto uv =
            glm::vec2(areaOfInterest->size - 1 /* small bias to avoid dirty edges */) /
            glm::vec2(areaOfInterest->framebuffer->framebuffer.supersampledSize());
        const glm::vec2 uvs[] = {
            { 0, 0},
            { uv.x, 0 },
            { 0, uv.y  },
            { uv.x, uv.y },
        };
        mRectangleVao.insertIfKeyMismatches(1, AArrayView(uvs), "backdrops");
    }

    auto& texture = areaOfInterest->framebuffer->rendertarget->texture();
    texture.bind();
    texture.setupClampToEdge();
    texture.setupLinear();

    uploadToShaderCommon();
    drawRectImpl(position, size);
}

OpenGLRenderer::FramebufferFromPool OpenGLRenderer::getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize) {
    auto ctx = dynamic_cast<OpenGLRenderingContext*>(getWindow()->getRenderingContext().get());
    if (!ctx) {
        return nullptr;
    }
    if (!ctx->framebuffer()) {
        return nullptr;
    }

    minRequiredSize.x = aui::bit_ceil(minRequiredSize.x);
    minRequiredSize.y = aui::bit_ceil(minRequiredSize.y);

    return aui::ptr::manage_unique(
        [&]() -> FramebufferWithTextureRT* {
          auto applicableSizeOnly =
              mFramebuffersForMultiPassEffectsPool | ranges::views::filter([&](const auto& fb) {
                return glm::all(glm::greaterThanEqual(fb->framebuffer.size(), minRequiredSize));
              });
          auto smallestFb = [](ranges::range auto& rng) {
            return ranges::min_element(rng, std::less<> {}, [](const auto& fb) {
              return fb->framebuffer.size().x * fb->framebuffer.size().y;
            });
          };

          auto release = [&](OffscreenFramebufferPool::iterator it) {
            auto object = std::exchange(*it, {});
            mFramebuffersForMultiPassEffectsPool.erase(it);
            return object.release();
          };

          if (auto it = smallestFb(applicableSizeOnly); it != applicableSizeOnly.end()) {
              return release(it.base());
          }

          if (auto it = smallestFb(mFramebuffersForMultiPassEffectsPool);
              it != mFramebuffersForMultiPassEffectsPool.end()) {
              (*it)->framebuffer.resize(glm::max((*it)->framebuffer.size(), minRequiredSize));
              return release(it);
          }

          auto object = std::make_unique<FramebufferWithTextureRT>();
          object->framebuffer.resize(minRequiredSize);
          object->rendertarget = _new<gl::TextureRenderTarget<gl::InternalFormat::RGBA8, gl::Type::UNSIGNED_BYTE, gl::Format::RGBA>>();
          object->framebuffer.attach(object->rendertarget, GL_COLOR_ATTACHMENT0);

          return object.release();
        }(),
        FramebufferBackToPool { this });
}

void OpenGLRenderer::FramebufferBackToPool::operator()(FramebufferWithTextureRT* framebuffer) const {
    renderer->mFramebuffersForMultiPassEffectsPool.emplace_back(framebuffer);
}
