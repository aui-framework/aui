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

OpenGLRenderer::TransientBuffer::TransientBuffer(GLenum target, size_t size) : mTarget(target), mSize(size) {
    glGenBuffers(1, &mHandle);
    glBindBuffer(mTarget, mHandle);
    glBufferData(mTarget, mSize, nullptr, GL_STREAM_DRAW);
}

OpenGLRenderer::TransientBuffer::~TransientBuffer() {
    if (mHandle) glDeleteBuffers(1, &mHandle);
}

size_t OpenGLRenderer::TransientBuffer::upload(const void* data, size_t size) {
    if (mOffset + size > mSize) {
        orphan();
    }
    size_t result = mOffset;
    glBufferSubData(mTarget, mOffset, size, data);
    mOffset += size;
    mOffset = (mOffset + 63) & ~63;
    return result;
}

void OpenGLRenderer::TransientBuffer::orphan() {
    glBindBuffer(mTarget, mHandle);
    glBufferData(mTarget, mSize, nullptr, GL_STREAM_DRAW);
    mOffset = 0;
}

void OpenGLRenderer::TransientBuffer::bind() {
    glBindBuffer(mTarget, mHandle);
}

void OpenGLRenderer::TransientBuffer::bindRange(GLuint index, size_t offset, size_t size) {
    glBindBufferRange(mTarget, index, mHandle, offset, size);
}

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

void OpenGLRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRectangles");
    setBlending(blending);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        for (auto rv : rectVertices) {
            vertices << Vertex{rv, {}, color};
        }
        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRectangles");
    setBlending(blending);
    mGradientShader->use();
    mGradientShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {0.f, 1.f}, color};
        vertices << Vertex{rectVertices[1], {1.f, 1.f}, color};
        vertices << Vertex{rectVertices[2], {0.f, 0.f}, color};
        vertices << Vertex{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRectangles");
    setBlending(blending);
    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {0.f, 1.f}, color};
        vertices << Vertex{rectVertices[1], {1.f, 1.f}, color};
        vertices << Vertex{rectVertices[2], {0.f, 0.f}, color};
        vertices << Vertex{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRoundedRectangles");
    setBlending(blending);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {0.f, 1.f}, color};
        vertices << Vertex{rectVertices[1], {1.f, 1.f}, color};
        vertices << Vertex{rectVertices[2], {0.f, 0.f}, color};
        vertices << Vertex{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
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
    if (v.instances.empty()) return;
    setBlending(blending);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {0.f, 1.f}, color};
        vertices << Vertex{rectVertices[1], {1.f, 1.f}, color};
        vertices << Vertex{rectVertices[2], {0.f, 0.f}, color};
        vertices << Vertex{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    setBlending(blending);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, transform);

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {0.f, 1.f}, color};
        vertices << Vertex{rectVertices[1], {1.f, 1.f}, color};
        vertices << Vertex{rectVertices[2], {0.f, 0.f}, color};
        vertices << Vertex{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) {
    GLDebugGroupLocal debugGroup("boxShadow");
    setBlending(blending);
    mBoxShadowShader->use();
    mBoxShadowShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = glm::vec4(v.color);

    Vertex vertices[] = {
        {rectVertices[0], {0.f, 1.f}, color},
        {rectVertices[1], {1.f, 1.f}, color},
        {rectVertices[2], {0.f, 0.f}, color},
        {rectVertices[3], {1.f, 0.f}, color}
    };
    GLuint indices[] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(Vertex) * 4);
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(GLuint) * 6);

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) {
    GLDebugGroupLocal debugGroup("boxShadowInner");
    setBlending(blending);
    mBoxShadowInnerShader->use();
    mBoxShadowInnerShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = glm::vec4(v.color);

    Vertex vertices[] = {
        {rectVertices[0], {0.f, 1.f}, color},
        {rectVertices[1], {1.f, 1.f}, color},
        {rectVertices[2], {0.f, 0.f}, color},
        {rectVertices[3], {1.f, 0.f}, color}
    };
    GLuint indices[] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(Vertex) * 4);
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(GLuint) * 6);

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("glyphs");
    setBlending(blending);
    mSymbolShader->use();
    mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<Vertex> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = glm::vec4(inst.color);
        
        vertices << Vertex{rectVertices[0], {inst.u1.x, inst.u2.y}, color};
        vertices << Vertex{rectVertices[1], {inst.u2.x, inst.u2.y}, color};
        vertices << Vertex{rectVertices[2], {inst.u1.x, inst.u1.y}, color};
        vertices << Vertex{rectVertices[3], {inst.u2.x, inst.u1.y}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void OpenGLRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

OpenGLRenderer::OpenGLRenderer() : 
    mVertexBuffer(GL_ARRAY_BUFFER, 1024 * 1024 * 2),
    mIndexBuffer(GL_ELEMENT_ARRAY_BUFFER, 1024 * 1024 * 1)
{
    useAuislShader<aui::sl_gen::basic::vsh::glsl120::Shader, aui::sl_gen::rect_solid::fsh::glsl120::Shader>(mSolidShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_textured::fsh::glsl120::Shader>(mTexturedShader);
    useAuislShader<aui::sl_gen::symbol::vsh::glsl120::Shader, aui::sl_gen::symbol::fsh::glsl120::Shader>(mSymbolShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_solid_rounded::fsh::glsl120::Shader>(mRoundedSolidShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::shadow::fsh::glsl120::Shader>(mBoxShadowShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::shadow_inner::fsh::glsl120::Shader>(mBoxShadowInnerShader);
    useAuislShader<aui::sl_gen::basic_uv::vsh::glsl120::Shader, aui::sl_gen::rect_gradient::fsh::glsl120::Shader>(mGradientShader);

    mBatchVao.bind();
    mVertexBuffer.bind();
    mIndexBuffer.bind();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
}

_<ITexture> OpenGLRenderer::createTexture(glm::u32vec2 size) {
    auto t = _new<OpenGLTexture2D>();
    t->texture().framebufferTex2D(size, gl::Type::UNSIGNED_BYTE);
    return t;
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
    auto rectVertices = getVerticesForRect(position, size);
    Vertex vertices[] = {
        {rectVertices[0], {0.f, 1.f}, AColor::WHITE},
        {rectVertices[1], {1.f, 1.f}, AColor::WHITE},
        {rectVertices[2], {0.f, 0.f}, AColor::WHITE},
        {rectVertices[3], {1.f, 0.f}, AColor::WHITE}
    };
    GLuint indices[] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(Vertex) * 4);
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(GLuint) * 6);

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(vOffset + offsetof(Vertex, color)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::identityUv() {
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
glm::mat4 OpenGLRenderer::getProjectionMatrix() const { return glm::mat4(1.0f); }
bool OpenGLRenderer::loadGL(GLLoadProc load_proc, bool es) { return true; }
bool OpenGLRenderer::loadGL(GLLoadProc load_proc) { return true; }
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
