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
#include <AUI/Render/FontAtlas.h>

#ifdef AUI_PLATFORM_WIN32
#include <windows.h>
#endif

bool OpenGLRenderer::mIsES = false;
int OpenGLRenderer::mGLSLVersion = 120;

namespace {
    std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 pos, glm::vec2 size) {
        return {
            pos,
            pos + glm::vec2(size.x, 0.f),
            pos + glm::vec2(0.f, size.y),
            pos + size
        };
    }

    AImage premultiplyImage(AImageView image) {
        AImage img(image.size(), image.format());
        auto comp = image.format() & APixelFormat::COMPONENT_BITS;
        bool hasAlpha = (comp == APixelFormat::RGBA || comp == APixelFormat::BGRA || comp == APixelFormat::ARGB);
        image.visit([&](const auto& source) {
            img.visit([&](auto& destination) {
                for (uint32_t y = 0; y < img.height(); ++y) {
                    for (uint32_t x = 0; x < img.width(); ++x) {
                        AColor c(source.get({x, y}));
                        if (hasAlpha) {
                            destination.set({x, y}, AFormattedColorConverter(c.premultiply()));
                        } else {
                            destination.set({x, y}, AFormattedColorConverter(c));
                        }
                    }
                }
            });
        });
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

    struct LineVertex {
        glm::vec2 position;
        float distance;
        float unused;
    };

    std::string getShaderPrefix(GLenum stage) {
        std::string prefix = "#version " + std::to_string(OpenGLRenderer::mGLSLVersion) + "\n";
        if (OpenGLRenderer::mGLSLVersion < 130) {
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
    }
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
void OpenGLRenderer::setBlending(const APaint& paint) {
    if (!glBlendFuncSeparate) return;
    glEnable(GL_BLEND);
    switch (paint.blending) {
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
            glBlendFuncSeparate(GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
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

void OpenGLRenderer::solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRectangles");
    setBlending(paint);
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

void OpenGLRenderer::gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRectangles");
    setBlending(paint);
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
void OpenGLRenderer::texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRectangles");
    setBlending(paint);
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

void OpenGLRenderer::solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRoundedRectangles");
    setBlending(paint);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    AVector<VertexRounded> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);

    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);

        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        vertices << VertexRounded{rectVertices[0], {0.f, 1.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[1], {1.f, 1.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[2], {0.f, 0.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[3], {1.f, 0.f}, color, outerSize};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, outerSize)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRoundedRectangles");
    setBlending(paint);
    mRoundedGradientShader->use();
    mRoundedGradientShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    mRoundedGradientShader->set(aui::ShaderUniforms::GRADIENT_MAT_UV, helper.matrix);

    AVector<VertexRoundedGradient> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);

    glm::vec4 c1 = v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? v.colors[1].color.premultiply() : (v.colors.size() > 0 ? v.colors[0].color.premultiply() : glm::vec4(0.f));

    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);

        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        vertices << VertexRoundedGradient{rectVertices[0], {0.f, 1.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[1], {1.f, 1.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[2], {0.f, 0.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[3], {1.f, 0.f}, color, outerSize, c1, c2};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, outerSize)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color1)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color2)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("texturedRoundedRectangles");
    setBlending(paint);
    mRoundedTexturedShader->use();
    mRoundedTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    static_cast<OpenGLTexture2D*>(v.texture.get())->bind();

    AVector<VertexRounded> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);

    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);

        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        vertices << VertexRounded{rectVertices[0], {0.f, 1.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[1], {1.f, 1.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[2], {0.f, 0.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[3], {1.f, 0.f}, color, outerSize};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, outerSize)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("rectangleBorders");
    setBlending(paint);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    AVector<VertexBasic> vertices;
    AVector<GLuint> indices;

    for (const auto& inst : v.instances) {
        glm::vec4 color = inst.color.premultiply();

        // 4 lines
        auto v1 = getVerticesForRect(inst.position, {inst.size.x, v.lineWidth});
        auto v2 = getVerticesForRect({inst.position.x, inst.position.y + inst.size.y - v.lineWidth}, {inst.size.x, v.lineWidth});
        auto v3 = getVerticesForRect({inst.position.x, inst.position.y + v.lineWidth}, {v.lineWidth, inst.size.y - v.lineWidth * 2.f});
        auto v4 = getVerticesForRect({inst.position.x + inst.size.x - v.lineWidth, inst.position.y + v.lineWidth}, {v.lineWidth, inst.size.y - v.lineWidth * 2.f});

        auto addRect = [&](const std::array<glm::vec2, 4>& r) {
            GLuint offset = (GLuint)vertices.size();
            for (int i = 0; i < 4; ++i) vertices << VertexBasic{r[i], color};
            indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
        };

        addRect(v1);
        addRect(v2);
        addRect(v3);
        addRect(v4);
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("roundedRectangleBorders");
    setBlending(paint);
    mRoundedSolidShaderBorder->use();
    mRoundedSolidShaderBorder->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    AVector<VertexRoundedBorder> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);

    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);

        float radius = v.radius;
        float maxRadius = std::min(inst.size.x, inst.size.y) * 0.5f;
        if (radius > maxRadius) {
            radius = maxRadius;
        }
        glm::vec2 outerSize = { 2.0f * radius / inst.size.x, 2.0f * radius / inst.size.y };

        float innerRadius = std::max(0.0f, radius - (float)v.borderWidth);
        glm::vec2 innerRectSize = inst.size - 2.0f * (float)v.borderWidth;

        glm::vec2 innerSize(0.f);
        glm::vec2 outerToInner(1.f);

        if (innerRectSize.x > 0.0f && innerRectSize.y > 0.0f) {
            innerSize = { 2.0f * innerRadius / innerRectSize.x, 2.0f * innerRadius / innerRectSize.y };
            outerToInner = inst.size / innerRectSize;
        } else {
            outerToInner = glm::vec2(1000.f);
        }

        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = inst.color.premultiply();

        vertices << VertexRoundedBorder{rectVertices[0], {0.f, 1.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[1], {1.f, 1.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[2], {0.f, 0.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[3], {1.f, 0.f}, color, outerSize, innerSize, outerToInner};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, outerSize)));
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, innerSize)));
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedBorder), (void*)(vOffset + offsetof(VertexRoundedBorder, outerToInner)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("boxShadow");
    setBlending(paint);
    mBoxShadowShader->use();
    mBoxShadowShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    float sigma = v.blurRadius / 2.f;
    float padding = v.blurRadius * 2.f;
    glm::vec2 pos = v.position - padding;
    glm::vec2 size = v.size + padding * 2.f;
    auto rectVertices = getVerticesForRect(pos, size);
    glm::vec4 color = v.color.premultiply();

    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, v.position);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, v.position + v.size);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, sigma);

    VertexBasic vertices[4];
    for (int i = 0; i < 4; ++i) vertices[i] = {rectVertices[i], color};
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));
    glDisableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLRenderer::boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("boxShadowInner");
    setBlending(paint);
    mBoxShadowInnerShader->use();
    mBoxShadowInnerShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    float sigma = v.blurRadius / 2.f;
    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = v.color.premultiply();

    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, v.position + v.offset + v.spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, v.position + v.size + v.offset - v.spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, sigma);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::OUTER_SIZE, glm::vec2(2.f * v.borderRadius) / v.size);

    VertexBasicUv vertices[4] = {
        {rectVertices[0], {0.f, 1.f}, color},
        {rectVertices[1], {1.f, 1.f}, color},
        {rectVertices[2], {0.f, 0.f}, color},
        {rectVertices[3], {1.f, 0.f}, color},
    };
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

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
void OpenGLRenderer::glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("glyphs");
    setBlending(paint);

    if (v.isSubpixel) {
        mSymbolShaderSubPixel->use();
        mSymbolShaderSubPixel->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    } else {
        mSymbolShader->use();
        mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    }
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

        vertices << VertexBasicUv{rectVertices[0], {inst.u1.x, inst.u2.y}, color};
        vertices << VertexBasicUv{rectVertices[1], {inst.u2.x, inst.u2.y}, color};
        vertices << VertexBasicUv{rectVertices[2], {inst.u1.x, inst.u1.y}, color};
        vertices << VertexBasicUv{rectVertices[3], {inst.u2.x, inst.u1.y}, color};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

_<IRenderer::IMultiStringCanvas> OpenGLRenderer::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::font_rendering::getFontEntryData(*this, getFontEntryDataCache(), style);
    return _new<aui::font_rendering::MultiStringCanvas>(*this, entryData, getCharacterDataCache(), style);
}
_<IRenderer::IPrerenderedString> OpenGLRenderer::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}

bool OpenGLRenderer::setupLineShader(const glm::mat4& transform, const ABorderStyle& style, float widthPx) {
    return std::visit(aui::lambda_overloaded{
        [&](const ABorderStyle::Solid&) {
            mSolidShader->use();
            mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
            return false;
        },
        [&](const ABorderStyle::Dashed& dashed) {
            mLineSolidDashedShader->use();
            mLineSolidDashedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
            float dashWidth = dashed.dashWidth.valueOr(1.f) * widthPx;
            float sumOfLengths = dashWidth + dashed.spaceBetweenDashes.valueOr(2.f) * widthPx;
            dashWidth *= mRenderScale;
            sumOfLengths *= mRenderScale;
            mLineSolidDashedShader->set(aui::ShaderUniforms::DIVIDER, sumOfLengths);
            mLineSolidDashedShader->set(aui::ShaderUniforms::THRESHOLD, dashWidth);
            return true;
        }
    }, style.value());
}

void OpenGLRenderer::lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.size() < 2) return;
    GLDebugGroupLocal debugGroup("lines");
    setBlending(paint);
    float widthPx = v.width.getValuePx();
    glLineWidth(widthPx);
    bool computeDistances = setupLineShader(transform, v.style, widthPx);

    AVector<VertexBasicUv> vertices;
    vertices.reserve(v.points.size());
    glm::vec4 color = paint.color.premultiply();

    float distanceAccumulator = 0.f;
    vertices << VertexBasicUv{v.points[0], {0.f, 0.f}, color};

    for (size_t i = 1; i < v.points.size(); ++i) {
        if (computeDistances) {
            distanceAccumulator += glm::distance(v.points[i-1], v.points[i]);
        }
        vertices << VertexBasicUv{v.points[i], {distanceAccumulator, 0.f}, color};
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)vertices.size());
}

void OpenGLRenderer::points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    GLDebugGroupLocal debugGroup("points");
    setBlending(paint);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    glPointSize(v.size.getValuePx());

    AVector<VertexBasic> vertices;
    vertices.reserve(v.points.size());
    glm::vec4 color = paint.color.premultiply();
    for (const auto& p : v.points) {
        vertices << VertexBasic{p, color};
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glEnableVertexAttribArray(2); // In VertexBasic, color is at index 1? Wait, no.
                                 // mSolidShader: {"pos", "color"} -> 0=pos, 1=color.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());
}

void OpenGLRenderer::lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    GLDebugGroupLocal debugGroup("lineBatches");
    setBlending(paint);
    float widthPx = v.width.getValuePx();
    glLineWidth(widthPx);
    bool computeDistances = setupLineShader(transform, v.style, widthPx);

    AVector<VertexBasicUv> vertices;
    vertices.reserve(v.points.size() * 2);
    glm::vec4 color = paint.color.premultiply();

    for (const auto& [p1, p2] : v.points) {
        float dist = computeDistances ? glm::distance(p1, p2) : 0.f;
        vertices << VertexBasicUv{p1, {0.f, 0.f}, color};
        vertices << VertexBasicUv{p2, {dist, 0.f}, color};
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));

    glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size());
}
void OpenGLRenderer::squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("squareSector");
    setBlending(paint);
    mSquareSectorShader->use();
    mSquareSectorShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);

    auto calculateLineMatrix = [](AAngleRadians angle) {
        auto s = glm::sin(angle.radians());
        auto c = glm::cos(angle.radians());
        return glm::mat3(c, 0.f, 0.f,
                         s, 0.f, 0.f,
                         -0.5f * (s + c), 0.f, 0.f);
    };

    mSquareSectorShader->set(aui::ShaderUniforms::M1, calculateLineMatrix(v.begin));
    mSquareSectorShader->set(aui::ShaderUniforms::M2, calculateLineMatrix(v.end));

    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = paint.color.premultiply();

    VertexBasic vertices[4];
    for (int i = 0; i < 4; ++i) vertices[i] = {rectVertices[i], color};
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}
void OpenGLRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {
    if (!glm::all(glm::greaterThan(size, glm::ivec2(0)))) return;
    if (backdrops.empty()) return;

    auto source = gl::Framebuffer::current();
    if (!source) return;

    struct AreaOfInterest {
        FramebufferFromPool framebuffer;
        glm::ivec2 size {};
    };

    AOptional<AreaOfInterest> areaOfInterest;

    auto initAreaOfInterestIfEmpty = [&](glm::ivec2 fbSize){
        if (areaOfInterest) return;

        auto fb = getFramebufferForMultiPassEffect(fbSize);
        source->bindForRead();
        fb->framebuffer.bindForWrite();

        auto vertices = getVerticesForRect(glm::vec2(position), glm::vec2(size));
        std::array<glm::ivec2, 4> transformedVertices{};
        for (size_t i = 0; i < 4; ++i) {
            auto ndc = glm::vec2(mProjectionMatrix * glm::vec4(vertices[i], 0, 1));
            transformedVertices[i] = glm::ivec2((ndc + 1.f) / 2.f * glm::vec2(source->supersampledSize()));
        }

        auto p1 = transformedVertices[0];
        auto p2 = transformedVertices[3];
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
            std::visit(aui::lambda_overloaded{
                [&](const ass::Backdrop::GaussianBlur& blur) {
                    initAreaOfInterestIfEmpty(size);
                    // Blur implementation would go here.
                    // For now, we just have the blitted area in areaOfInterest.
                },
                [&](const auto&) {}
            }, backdrop);
        }
    }

    if (!areaOfInterest) return;

    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix);
    mTexturedShader->set(aui::ShaderUniforms::COLOR, glm::vec4(1.f));
    static_cast<OpenGLTexture2D*>(areaOfInterest->framebuffer->renderTarget.get())->bind();

    auto rectVertices = getVerticesForRect(glm::vec2(position), glm::vec2(size));
    auto uvSize = glm::vec2(areaOfInterest->size) / glm::vec2(areaOfInterest->framebuffer->framebuffer.size());

    VertexBasicUv vertices[4] = {
        {rectVertices[0], {0.f, uvSize.y}, glm::vec4(1.f)},
        {rectVertices[1], {uvSize.x, uvSize.y}, glm::vec4(1.f)},
        {rectVertices[2], {0.f, 0.f}, glm::vec4(1.f)},
        {rectVertices[3], {uvSize.x, 0.f}, glm::vec4(1.f)},
    };
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

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

OpenGLRenderer::FramebufferFromPool OpenGLRenderer::getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize) {
    minRequiredSize.x = aui::bit_ceil(minRequiredSize.x);
    minRequiredSize.y = aui::bit_ceil(minRequiredSize.y);

    return aui::ptr::manage_unique(
        [&]() -> FramebufferWithTextureRT* {
            for (auto it = mFramebuffersForMultiPassEffectsPool.begin(); it != mFramebuffersForMultiPassEffectsPool.end(); ++it) {
                if (glm::all(glm::greaterThanEqual((*it)->framebuffer.size(), minRequiredSize))) {
                    auto res = std::move(*it);
                    mFramebuffersForMultiPassEffectsPool.erase(it);
                    return res.release();
                }
            }
            if (!mFramebuffersForMultiPassEffectsPool.empty()) {
                auto res = std::move(mFramebuffersForMultiPassEffectsPool.back());
                mFramebuffersForMultiPassEffectsPool.pop_back();
                res->framebuffer.resize(glm::max(res->framebuffer.size(), minRequiredSize));
                return res.release();
            }

            auto object = std::make_unique<FramebufferWithTextureRT>();
            object->framebuffer.resize(minRequiredSize);
            object->renderTarget = _cast<gl::Framebuffer::IRenderTarget>(createTexture(minRequiredSize));
            object->framebuffer.attach(object->renderTarget, GL_COLOR_ATTACHMENT0);
            return object.release();
        }(),
        FramebufferBackToPool { this });
}

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
