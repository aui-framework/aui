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

#include "OpenGLRenderer.h"
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Traits/callables.h>
#include <AUI/GL/gl.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/GL/ShaderUniforms.h>
#include <AUI/Render/RenderHints.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <AUI/GL/GLDebug.h>
#include <AUI/GL/State.h>
#include <AUI/Platform/OpenGLRenderingContext.h>
#include <AUI/GL/RenderTarget/TextureRenderTarget.h>
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
#include <range/v3/all.hpp>

#define LOG_TAG "OpenGLRenderer"

// ---------------------------------------------------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------------------------------------------------

struct GLDebugGroupLocal {
    GLDebugGroupLocal(const char* name) {
        if (glPushDebugGroup) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
        }
    }
    ~GLDebugGroupLocal() {
        if (glPopDebugGroup) {
            glPopDebugGroup();
        }
    }
};

static std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 position, glm::vec2 size) {
    float x = position.x;
    float y = position.y;
    float w = x + size.x;
    float h = y + size.y;
    return {
        glm::vec2{x, h},
        glm::vec2{w, h},
        glm::vec2{x, y},
        glm::vec2{w, y}
    };
}

template<typename Vertex, typename Fragment>
inline void useAuislShader(AOptional<gl::Program>& out) {
    out.emplace();
    out->loadBoth(Vertex::code(), Fragment::code());
    Vertex::setup(out->handle());
    Fragment::setup(out->handle());
    out->compile();
}

// ---------------------------------------------------------------------------------------------------------------------
// Text rendering implementations
// ---------------------------------------------------------------------------------------------------------------------

struct CharacterGlyph {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 u1;
    glm::vec2 u2;
    AColor color;
};

class OpenGLPrerenderedString : public IRenderer::IPrerenderedString {
private:
    OpenGLRenderer* mRenderer;
    AVector<CharacterGlyph> mGlyphs;
    int mTextWidth;
    int mTextHeight;
    OpenGLRenderer::FontEntryData* mEntryData;

public:
    OpenGLPrerenderedString(OpenGLRenderer* renderer,
                            AVector<CharacterGlyph> glyphs,
                            int textWidth,
                            int textHeight,
                            OpenGLRenderer::FontEntryData* entryData) :
        mRenderer(renderer),
        mGlyphs(std::move(glyphs)),
        mTextWidth(textWidth),
        mTextHeight(textHeight),
        mEntryData(entryData) {}

    void draw(ACanvas& canvas) override {
        if (mEntryData->isTextureInvalid) {
            if (auto img = mEntryData->texturePacker.getImage()) {
                mEntryData->texture->setImage(*img);
            }
            mEntryData->isTextureInvalid = false;
        }
        for (const auto& g : mGlyphs) {
            canvas.glyphRect(mEntryData->texture, g.position, g.size, g.u1, g.u2, g.color);
        }
    }

    int getWidth() override { return mTextWidth; }
    int getHeight() override { return mTextHeight; }
};

class OpenGLMultiStringCanvas : public IRenderer::IMultiStringCanvas {
private:
    AVector<CharacterGlyph> mGlyphs;
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
    }

    template<class UnicodeString>
    void addStringT(const glm::ivec2& position, UnicodeString text) noexcept {
        auto& font = mFontStyle.font;
        auto& texturePacker = mEntryData->texturePacker;
        auto fe = mFontStyle.getFontEntry();

        const bool hasKerning = font->isHasKerning();

        int advanceX = position.x;
        int advanceY = position.y;
        float advance = (float)advanceX;
        for (auto i = text.begin(); i != text.end(); ++i) {
            AChar c = *i;
            if (c == ' ') {
                advance += mFontStyle.getSpaceWidth();
            } else if (c == '\n') {
                advanceX = (glm::max)(advanceX, int(glm::ceil(advance)));
                advance = (float)position.x;
                advanceY += mFontStyle.getLineHeight();
            } else {
                AFont::Character& ch = font->getCharacter(fe, c);
                if (ch.empty()) {
                    advance += mFontStyle.getSpaceWidth();
                    continue;
                }
                
                int posX = (int)advance + ch.horizontal.bearing.x;
                int posY = advanceY - ch.horizontal.bearing.y;
                int width = ch.image->width();
                int height = ch.image->height();

                glm::vec4 uv;
                if (ch.rendererData == nullptr) {
                    uv = texturePacker.insert(*ch.image);
                    const float BIAS = 0.1f;
                    uv.x += BIAS; uv.y += BIAS; uv.z -= BIAS; uv.w -= BIAS;
                    mRenderer->mCharData.push_back(OpenGLRenderer::CharacterData{uv});
                    ch.rendererData = &mRenderer->mCharData.last();
                    mEntryData->isTextureInvalid = true;
                } else {
                    uv = reinterpret_cast<OpenGLRenderer::CharacterData*>(ch.rendererData)->uv;
                }

                mGlyphs.push_back({
                    glm::vec2(posX, posY),
                    glm::vec2(width, height),
                    glm::vec2(uv.x, uv.y),
                    glm::vec2(uv.z, uv.w),
                    AColor::WHITE
                });

                if (hasKerning) {
                    auto next = std::next(i);
                    if (next != text.end()) {
                        auto kerning = font->getKerning(c, *next);
                        advance += (float)kerning.x;
                    }
                }
                advance += (float)ch.horizontal.advance;
            }
        }
        mAdvanceX = (glm::max)(mAdvanceX, (glm::max)(advanceX, int(glm::ceil(advance))));
        mAdvanceY = advanceY + mFontStyle.getLineHeight();
    }

    void addString(const glm::ivec2& position, AStringView text) noexcept override { addStringT(position, text.utf8()); }
    void addString(const glm::ivec2& position, std::u32string_view text) noexcept override { addStringT(position, text); }

    _<IRenderer::IPrerenderedString> finalize() noexcept override {
        return _new<OpenGLPrerenderedString>(mRenderer, std::move(mGlyphs), mAdvanceX, mAdvanceY, mEntryData);
    }
};

// ---------------------------------------------------------------------------------------------------------------------
// IRendererBackend implementation (Stateless)
// ---------------------------------------------------------------------------------------------------------------------

void OpenGLRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRectangles");
    setBlending(blending);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<glm::vec2> vertices;
    AVector<glm::vec4> colors;
    AVector<GLuint> indices;
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        for (auto rv : rectVertices) vertices << rv;
        glm::vec4 color = glm::vec4(inst.color);
        for (int j = 0; j < 4; ++j) colors << color;
        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }
    mBatchVao.bind();
    mBatchVao.insert(0, AArrayView(vertices), "solidRectangles/positions");
    mBatchVao.insert(2, AArrayView(colors), "solidRectangles/colors");
    mBatchVao.indices(AArrayView(indices));
    mBatchVao.drawElements();
}

void OpenGLRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRectangles");
    setBlending(blending);
    mGradientShader->use();
    mGradientShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    for (const auto& inst : v.instances) {
        auto vertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        glm::vec4 colors[4] = {color, color, color, color};
        mRectangleVao.insert(0, AArrayView(vertices), "gradientRectangles/positions");
        mRectangleVao.insert(2, AArrayView(colors), "gradientRectangles/colors");
        mRectangleVao.drawElements();
    }
}
void OpenGLRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRectangles");
    setBlending(blending);
    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();
    for (const auto& inst : v.instances) {
        auto vertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        glm::vec4 colors[4] = {color, color, color, color};
        mRectangleVao.insert(0, AArrayView(vertices), "texturedRectangles/positions");
        mRectangleVao.insert(2, AArrayView(colors), "texturedRectangles/colors");
        mRectangleVao.drawElements();
    }
}
void OpenGLRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRoundedRectangles");
    setBlending(blending);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    for (const auto& inst : v.instances) {
        auto vertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        glm::vec4 colors[4] = {color, color, color, color};
        mRectangleVao.insert(0, AArrayView(vertices), "solidRoundedRectangles/positions");
        mRectangleVao.insert(2, AArrayView(colors), "solidRoundedRectangles/colors");
        mRectangleVao.drawElements();
    }
}
void OpenGLRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    setBlending(blending);
}
void OpenGLRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    setBlending(blending);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();
}
void OpenGLRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) {
    setBlending(blending);
    for (const auto& inst : v.instances) {
        auto vertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        glm::vec4 colors[4] = {color, color, color, color};
        mRectangleVao.insert(0, AArrayView(vertices), "rectangleBorders/positions");
        mRectangleVao.insert(2, AArrayView(colors), "rectangleBorders/colors");
        mRectangleVao.drawElements();
    }
}
void OpenGLRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) {
    setBlending(blending);
    for (const auto& inst : v.instances) {
        auto vertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        glm::vec4 colors[4] = {color, color, color, color};
        mRectangleVao.insert(0, AArrayView(vertices), "roundedRectangleBorders/positions");
        mRectangleVao.insert(2, AArrayView(colors), "roundedRectangleBorders/colors");
        mRectangleVao.drawElements();
    }
}
void OpenGLRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) {
    GLDebugGroupLocal debugGroup("boxShadow");
    setBlending(blending);
    mBoxShadowShader->use();
    mBoxShadowShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    auto vertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = glm::vec4(v.color);
    glm::vec4 colors[4] = {color, color, color, color};
    mRectangleVao.insert(0, AArrayView(vertices), "boxShadow/positions");
    mRectangleVao.insert(2, AArrayView(colors), "boxShadow/colors");
    mRectangleVao.drawElements();
}
void OpenGLRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) {
    GLDebugGroupLocal debugGroup("boxShadowInner");
    setBlending(blending);
    mBoxShadowInnerShader->use();
    mBoxShadowInnerShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    auto vertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = glm::vec4(v.color);
    glm::vec4 colors[4] = {color, color, color, color};
    mRectangleVao.insert(0, AArrayView(vertices), "boxShadowInner/positions");
    mRectangleVao.insert(2, AArrayView(colors), "boxShadowInner/colors");
    mRectangleVao.drawElements();
}
void OpenGLRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("glyphs");
    setBlending(blending);
    mSymbolShader->use();
    mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<glm::vec2> vertices;
    AVector<glm::vec2> uvs;
    AVector<glm::vec4> colors;
    AVector<GLuint> indices;
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        for (auto rv : rectVertices) vertices << rv;
        uvs << glm::vec2{inst.u1.x, inst.u2.y} << glm::vec2{inst.u2.x, inst.u2.y} 
            << glm::vec2{inst.u1.x, inst.u1.y} << glm::vec2{inst.u2.x, inst.u1.y};
        
        glm::vec4 color = glm::vec4(inst.color);
        for (int j = 0; j < 4; ++j) colors << color;

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }
    mBatchVao.bind();
    mBatchVao.insert(0, AArrayView(vertices), "glyphs/positions");
    mBatchVao.insert(1, AArrayView(uvs), "glyphs/uvs");
    mBatchVao.insert(2, AArrayView(colors), "glyphs/colors");
    mBatchVao.indices(AArrayView(indices));
    mBatchVao.drawElements();
}
void OpenGLRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void OpenGLRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

// ---------------------------------------------------------------------------------------------------------------------
// Common / Internal
// ---------------------------------------------------------------------------------------------------------------------

OpenGLRenderer::OpenGLRenderer() : mTexturePool([]{ return std::unique_ptr<ITexture>(new OpenGLTexture2D()); }) {
    useAuislShader<aui::sl_gen::basic::vsh::glsl120::Shader, aui::sl_gen::rect_solid::fsh::glsl120::Shader>(mSolidShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_textured::fsh::glsl120::Shader>(mTexturedShader);
    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader, aui::sl_gen::symbol::fsh::glsl120::Shader>(mSymbolShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_solid_rounded::fsh::glsl120::Shader>(mRoundedSolidShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::shadow::fsh::glsl120::Shader>(mBoxShadowShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::shadow_inner::fsh::glsl120::Shader>(mBoxShadowInnerShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_gradient::fsh::glsl120::Shader>(mGradientShader);
    
    constexpr GLuint INDICES[] = {0, 1, 2, 2, 1, 3};
    mRectangleVao.indices(INDICES);
}
void OpenGLRenderer::setBlending(Blending blending) {
    if (glBlendFuncSeparate) {
        switch (blending) {
            case Blending::NORMAL: glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE); return;
            case Blending::INVERSE_DST: glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE); return;
            case Blending::ADDITIVE: glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE); return;
            case Blending::INVERSE_SRC: glBlendFuncSeparate(GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE); return;
        }
    }
}
void OpenGLRenderer::beginPaint(glm::uvec2 windowSize) {}
void OpenGLRenderer::endPaint() {}
void OpenGLRenderer::drawRectImpl(glm::vec2 position, glm::vec2 size) {
    mRectangleVao.bind();
    mRectangleVao.insert(0, AArrayView(getVerticesForRect(position, size)), "drawRectImpl");
    mRectangleVao.drawElements();
}
void OpenGLRenderer::identityUv() {
    static constexpr glm::vec2 uvs[] = { {0.f, 1.f}, {1.f, 1.f}, {0.f, 0.f}, {1.f, 0.f} };
    mRectangleVao.insert(1, AArrayView(uvs), "identityUv");
}
_<IRenderer::IPrerenderedString> OpenGLRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    OpenGLMultiStringCanvas c(this, fs);
    c.addString(position, text);
    return c.finalize();
}
_<IRenderer::IMultiStringCanvas> OpenGLRenderer::newMultiStringCanvas(const AFontStyle& style) {
    return _new<OpenGLMultiStringCanvas>(this, style);
}
_unique<IRenderViewToTexture> OpenGLRenderer::newRenderViewToTexture() noexcept { return nullptr; }
_unique<ITexture> OpenGLRenderer::createNewTexture() { return _unique<ITexture>(new OpenGLTexture2D()); }
glm::mat4 OpenGLRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }
bool OpenGLRenderer::loadGL(GLLoadProc load_proc, bool es) { return true; }
bool OpenGLRenderer::loadGL(GLLoadProc load_proc) { return true; }
void OpenGLRenderer::bindTemporaryVao() const noexcept { mRectangleVao.bind(); }
uint32_t OpenGLRenderer::getDefaultFb() const noexcept { return 0; }
OpenGLRenderer::FontEntryData* OpenGLRenderer::getFontEntryData(const AFontStyle& fontStyle) {
    auto fe = fontStyle.getFontEntry();
    if (fe.second.rendererData == nullptr) {
        mFontEntryData.emplace_back(this);
        fe.second.rendererData = &mFontEntryData.last();
    }
    return reinterpret_cast<FontEntryData*>(fe.second.rendererData);
}
void OpenGLRenderer::FramebufferBackToPool::operator()(FramebufferWithTextureRT* framebuffer) const {}
OpenGLRenderer::FramebufferFromPool OpenGLRenderer::getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize) { return nullptr; }
