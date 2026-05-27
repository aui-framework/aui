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
#include <AUI/Render/IRenderer.h>
#include <AUI/Traits/callables.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Traits/values.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Image/AImage.h>
#include <AUI/GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/ABuiltinFiles.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/GL/ShaderUniforms.h>
#include <AUI/Render/Brush/Gradient.h>

#ifdef AUI_PLATFORM_WIN32
#include <windows.h>
#endif

bool OpenGLRenderer::mIsES = false;
int OpenGLRenderer::mGLSLVersion = 120;

namespace {
    AVector<glm::vec2> getVerticesForRect(glm::vec2 pos, glm::vec2 size) {
        return {
            pos,
            pos + glm::vec2(size.x, 0.f),
            pos + glm::vec2(0.f, size.y),
            pos + size
        };
    }

    AImage premultiplyImage(AImageView image) {
        auto comp = image.format() & APixelFormat::COMPONENT_BITS;
        bool hasAlpha = (comp == APixelFormat::RGBA || comp == APixelFormat::BGRA || comp == APixelFormat::ARGB);
        if (!hasAlpha) {
            return AImage(image);
        }
        AImage img(image);
        for (uint32_t y = 0; y < img.height(); ++y) {
            for (uint32_t x = 0; x < img.width(); ++x) {
                AColor c = img.get({x, y});
                img.set({x, y}, c.premultiply());
            }
        }
        return img;
    }

    struct GLDebugGroupLocal {
        GLDebugGroupLocal(const char* name) {
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
            if (glPushDebugGroup) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
#endif
        }
        ~GLDebugGroupLocal() {
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
            if (glPopDebugGroup) glPopDebugGroup();
#endif
        }
    };
}

OpenGLRenderer::OpenGLRenderer() : 
    mVertexBuffer(GL_ARRAY_BUFFER, 2 * 1024 * 1024), 
    mIndexBuffer(GL_ELEMENT_ARRAY_BUFFER, 1024 * 1024) 
{
    auto readShader = [](const AString& name) {
        auto buffer = AByteBuffer::fromStream(ABuiltinFiles::open(name + ".glsl"));
        return std::string(buffer.begin(), buffer.end());
    };

    auto useShader = [&](AOptional<gl::Program>& out, const AString& vertex, const AString& fragment, std::initializer_list<const char*> attrs) {
        auto getPrefix = [](GLenum stage) -> std::string {
            std::string prefix = "#version " + std::to_string(mGLSLVersion) + "\n";
            if (mGLSLVersion < 130) {
                if (stage == GL_VERTEX_SHADER) {
                    prefix += "#define in attribute\n#define out varying\n";
                } else {
                    prefix += "#define in varying\n#define fragColor gl_FragColor\n";
                }
            } else {
                if (stage == GL_FRAGMENT_SHADER) {
                    prefix += "out vec4 fragColor;\n";
                }
            }
            prefix += R"(
#ifdef GL_ES
precision mediump float;
#define mediump
#define highp
#else
#if __VERSION__ >= 130
precision highp float;
precision highp int;
#endif
#define mediump
#define highp
#endif
)";
            return prefix;
        };

        out.emplace();
        out->loadVertexShader(getPrefix(GL_VERTEX_SHADER) + readShader(vertex), { .custom = true });
        out->loadFragmentShader(getPrefix(GL_FRAGMENT_SHADER) + readShader(fragment), { .custom = true });
        
        int i = 0;
        for (auto attr : attrs) {
            out->bindAttribute(i++, attr);
        }
        out->compile();
    };

    useShader(mSolidShader, "basic.vsh", "rect_solid.fsh", {"pos", "color"});
    useShader(mBoxShadowShader, "basic_uv.vsh", "shadow.fsh", {"pos", "uv", "color"});
    useShader(mBoxShadowInnerShader, "basic_uv.vsh", "shadow_inner.fsh", {"pos", "uv", "color"});
    useShader(mRoundedSolidShader, "basic_uv.vsh", "rect_solid_rounded.fsh", {"pos", "uv", "color"});
    useShader(mRoundedSolidShaderBorder, "basic_uv.vsh", "border_rounded.fsh", {"pos", "uv", "color"});
    useShader(mGradientShader, "basic_uv.vsh", "rect_gradient.fsh", {"pos", "uv", "color"});
    useShader(mRoundedGradientShader, "basic_uv.vsh", "rect_gradient_rounded.fsh", {"pos", "uv", "color"});
    useShader(mTexturedShader, "basic_uv.vsh", "rect_textured.fsh", {"pos", "uv", "color"});
    useShader(mRoundedTexturedShader, "basic_uv.vsh", "rect_textured_rounded.fsh", {"pos", "uv", "color"});
    useShader(mUnblendShader, "basic_uv.vsh", "rect_unblend.fsh", {"pos", "uv", "color"});
    useShader(mSquareSectorShader, "basic_uv.vsh", "square_sector.fsh", {"pos", "uv", "color"});
    useShader(mSymbolShader, "symbol.vsh", "symbol.fsh", {"pos", "uv", "color"});
    useShader(mSymbolShaderSubPixel, "symbol.vsh", "symbol_sub.fsh", {"pos", "uv", "color"});
    useShader(mLineSolidDashedShader, "basic_uv.vsh", "line_solid_dashed.fsh", {"pos", "uv", "color"});

    mBatchVao.bind();
    mVertexBuffer.bind();
    mIndexBuffer.bind();

    // Basic (pos=0, color=1)
    // BasicUv/Symbol (pos=0, uv=1, color=2)
    glEnableVertexAttribArray(0); // pos
    glEnableVertexAttribArray(1); // uv
    glEnableVertexAttribArray(2); // color
}

void OpenGLTexture2D::upload(AImageView image) {
    mTexture.tex2D(premultiplyImage(image));
}

_<ITexture> OpenGLRenderer::createTexture(glm::u32vec2 size, APixelFormat format) {
    auto t = _new<OpenGLTexture2D>();
    t->texture().tex2D(AImage(size, format));
    return t;
}
void OpenGLRenderer::setBlending(Blending blending) {
    if (!glBlendFuncSeparate) return;
    glEnable(GL_BLEND);
    switch (blending) {
        case Blending::NORMAL:
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case Blending::INVERSE_DST:
            glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
            break;
        case Blending::ADDITIVE:
            glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
            break;
        case Blending::INVERSE_SRC:
            glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
            break;
    }
}
void OpenGLRenderer::beginPaint(glm::uvec2 windowSize) {
    mViewportSize = windowSize;
    mProjectionMatrix = glm::ortho(0.f, (float)mViewportSize.x, (float)mViewportSize.y, 0.f, -1.f, 1.f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}
void OpenGLRenderer::endPaint() {}
void OpenGLRenderer::setWindow(ASurface* window) {
    mWindow = window;
    if (mWindow) {
        mViewportSize = mWindow->getSize();
        mProjectionMatrix = glm::ortho(0.f, (float)mViewportSize.x, (float)mViewportSize.y, 0.f, -1.f, 1.f);
    }
}
_unique<IRenderViewToTexture> OpenGLRenderer::newRenderViewToTexture() noexcept { return nullptr; }
glm::mat4 OpenGLRenderer::getProjectionMatrix() const {
    return mProjectionMatrix;
}
bool OpenGLRenderer::loadGL(GLLoadProc load_proc, bool es) {
    mIsES = es;
    if (mIsES) {
        if (GLAD_GL_ES_VERSION_3_0) mGLSLVersion = 300;
        else mGLSLVersion = 100;
    } else {
        if (GLAD_GL_VERSION_3_3) mGLSLVersion = 330;
        else if (GLAD_GL_VERSION_3_0) mGLSLVersion = 130;
        else mGLSLVersion = 120;
    }
    return true;
}

bool OpenGLRenderer::loadGL(GLLoadProc load_proc) { return loadGL(load_proc, false); }
uint32_t OpenGLRenderer::getDefaultFb() const noexcept { return 0; }
void OpenGLRenderer::FramebufferBackToPool::operator()(FramebufferWithTextureRT* framebuffer) const {}

void OpenGLRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRectangles");
    setBlending(blending);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    AVector<VertexBasic> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();
        for (int j = 0; j < 4; ++j) {
            vertices << VertexBasic{rectVertices[j], color};
        }
        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRectangles");
    setBlending(blending);
    mGradientShader->use();
    mGradientShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    mGradientShader->set(aui::ShaderUniforms::GRADIENT_MAT_UV, helper.matrix);

    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));
    mGradientShader->set(aui::ShaderUniforms::COLOR1, c1);
    mGradientShader->set(aui::ShaderUniforms::COLOR2, c2);

    AVector<VertexBasicUv> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();
        
        vertices << VertexBasicUv{rectVertices[0], {0.f, 1.f}, color};
        vertices << VertexBasicUv{rectVertices[1], {1.f, 1.f}, color};
        vertices << VertexBasicUv{rectVertices[2], {0.f, 0.f}, color};
        vertices << VertexBasicUv{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRectangles");
    setBlending(blending);
    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<VertexBasicUv> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();
        
        vertices << VertexBasicUv{rectVertices[0], {0.f, 1.f}, color};
        vertices << VertexBasicUv{rectVertices[1], {1.f, 1.f}, color};
        vertices << VertexBasicUv{rectVertices[2], {0.f, 0.f}, color};
        vertices << VertexBasicUv{rectVertices[3], {1.f, 0.f}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRoundedRectangles");
    setBlending(blending);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    for (const auto& inst : v.instances) {
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };
        mRoundedSolidShader->set(aui::ShaderUniforms::OUTER_SIZE, outerSize);

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        VertexBasicUv vertices[4] = {
            {rectVertices[0], {0.f, 1.f}, color},
            {rectVertices[1], {1.f, 1.f}, color},
            {rectVertices[2], {0.f, 0.f}, color},
            {rectVertices[3], {1.f, 0.f}, color}
        };
        GLuint indices[6] = { 0, 1, 2, 2, 1, 3 };

        size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
        size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

        mBatchVao.bind();
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
    }
}

void OpenGLRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRoundedRectangles");
    setBlending(blending);
    mRoundedGradientShader->use();
    mRoundedGradientShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    mRoundedGradientShader->set(aui::ShaderUniforms::GRADIENT_MAT_UV, helper.matrix);

    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));
    mRoundedGradientShader->set(aui::ShaderUniforms::COLOR1, c1);
    mRoundedGradientShader->set(aui::ShaderUniforms::COLOR2, c2);

    for (const auto& inst : v.instances) {
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };
        mRoundedGradientShader->set(aui::ShaderUniforms::OUTER_SIZE, outerSize);

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        VertexBasicUv vertices[4] = {
            {rectVertices[0], {0.f, 1.f}, color},
            {rectVertices[1], {1.f, 1.f}, color},
            {rectVertices[2], {0.f, 0.f}, color},
            {rectVertices[3], {1.f, 0.f}, color}
        };
        GLuint indices[6] = { 0, 1, 2, 2, 1, 3 };

        size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
        size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

        mBatchVao.bind();
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
    }
}

void OpenGLRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRoundedRectangles");
    setBlending(blending);
    mRoundedTexturedShader->use();
    mRoundedTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    for (const auto& inst : v.instances) {
        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };
        mRoundedTexturedShader->set(aui::ShaderUniforms::OUTER_SIZE, outerSize);

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        VertexBasicUv vertices[4] = {
            {rectVertices[0], {0.f, 1.f}, color},
            {rectVertices[1], {1.f, 1.f}, color},
            {rectVertices[2], {0.f, 0.f}, color},
            {rectVertices[3], {1.f, 0.f}, color}
        };
        GLuint indices[6] = { 0, 1, 2, 2, 1, 3 };

        size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
        size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

        mBatchVao.bind();
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
    }
}
void OpenGLRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("glyphs");
    setBlending(blending);
    
    if (v.isSubpixel) {
        mSymbolShaderSubPixel->use();
        mSymbolShaderSubPixel->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    } else {
        mSymbolShader->use();
        mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    }
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<VertexSymbol> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();
        
        vertices << VertexSymbol{rectVertices[0], {inst.u1.x, inst.u2.y}, color};
        vertices << VertexSymbol{rectVertices[1], {inst.u2.x, inst.u2.y}, color};
        vertices << VertexSymbol{rectVertices[2], {inst.u1.x, inst.u1.y}, color};
        vertices << VertexSymbol{rectVertices[3], {inst.u2.x, inst.u1.y}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexSymbol), (void*)(vOffset + offsetof(VertexSymbol, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexSymbol), (void*)(vOffset + offsetof(VertexSymbol, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexSymbol), (void*)(vOffset + offsetof(VertexSymbol, color)));
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) {}
void OpenGLRenderer::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {}
void OpenGLRenderer::backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {}

OpenGLRenderer::TransientBuffer::TransientBuffer(GLenum target, size_t size) : mTarget(target), mSize(size) {
    glGenBuffers(1, &mHandle);
    bind();
    glBufferData(target, size, nullptr, GL_STREAM_DRAW);
}
OpenGLRenderer::TransientBuffer::~TransientBuffer() {
    if (mHandle) glDeleteBuffers(1, &mHandle);
}
void OpenGLRenderer::TransientBuffer::bind() {
    glBindBuffer(mTarget, mHandle);
}
void OpenGLRenderer::TransientBuffer::orphan() {
    bind();
    glBufferData(mTarget, mSize, nullptr, GL_STREAM_DRAW);
    mOffset = 0;
}
size_t OpenGLRenderer::TransientBuffer::upload(const void* data, size_t size) {
    if (mOffset + size > mSize) orphan();
    size_t res = mOffset;
    bind();
    glBufferSubData(mTarget, mOffset, size, data);
    mOffset += (size + 15) & ~15; // align to 16 bytes
    return res;
}
