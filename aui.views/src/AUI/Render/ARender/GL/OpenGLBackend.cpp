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

#include "OpenGLBackend.hpp"

#include "TextureFormatRecognition.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/Traits/callables.h>
#include <AUI/Platform/ASurface.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <AUI/Traits/values.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Image/AImage.h>
#include <AUI/Render/ARender/GL/gl.h>
#include <AUI/Render/ARender/GL/State.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/ABuiltinFiles.h>
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Render/ARender/GL/ShaderUniforms.h>
#include <AUI/Hash.h>
#include <AUI/Render/Brush/Gradient.h>
#include <AUI/Render/FontAtlas.hpp>
#include <AUI/Platform/AFontManager.h>
#include <AUI/View/AAbstractLabel.h>
#include <AUI/Performance/APerformanceSection.h>
#include <AUI/Render/ARender/CommonOffscreenRenderPass.h>
#include <AUI/Util/GaussianKernel.h>
#include <array>
#include <bit>
#include <cmath>

#ifdef AUI_PLATFORM_WIN32
#include <windows.h>
#endif

bool OpenGLBackend::mIsES = false;
int OpenGLBackend::mGLSLVersion = 120;

namespace {

class OpenGLFramebufferTexture : public ITexture {
public:
    OpenGLFramebufferTexture(uint32_t handle, glm::uvec2 size) : mHandle(handle), mSize(size) {
        mOrigin = TextureOrigin::TOP_LEFT;
    }
    glm::u32vec2 getSize() const override { return mSize; }
    APixelFormat getFormat() const override { return APixelFormat::R8G8B8A8_UNORM; }
    void upload(AImageView image) override {}

    void bind() { gl::State::bindTexture(GL_TEXTURE_2D, mHandle); }
    void bind(uint32_t unit) {
        gl::State::activeTexture(unit);
        gl::State::bindTexture(GL_TEXTURE_2D, mHandle);
    }

    uint32_t handle() const noexcept { return mHandle; }
private:
    uint32_t mHandle;
    glm::uvec2 mSize;
};

void bindTexture(ITexture* texture, uint32_t unit = 0) {
    if (auto t = dynamic_cast<OpenGLTexture2D*>(texture)) {
        t->bind(unit);
    } else if (auto t = dynamic_cast<OpenGLFramebufferTexture*>(texture)) {
        t->bind(unit);
    }
}

std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 pos, glm::vec2 size) {
    return {
        pos,
        pos + glm::vec2(size.x, 0.f),
        pos + glm::vec2(0.f, size.y),
        pos + size
    };
}

struct GLDebugGroupLocal {
    GLDebugGroupLocal(const char* name) {
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
        if (glPushDebugGroup) {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
        }
#endif
    }
    ~GLDebugGroupLocal() {
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
        if (glPopDebugGroup) {
            glPopDebugGroup();
        }
#endif
    }
};

struct VertexBasic {
    glm::vec2 pos;
    glm::vec4 color;
};
struct VertexBasicUv {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
};
struct VertexRounded {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec2 outerSize;
};
struct VertexRoundedBorder {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec2 outerSize;
    glm::vec2 innerSize;
    glm::vec2 outerToInner;
};
struct VertexRoundedGradient {
    glm::vec2 pos;
    glm::vec2 uv;
    glm::vec4 color;
    glm::vec2 outerSize;
    glm::vec4 color1;
    glm::vec4 color2;
};

const gl::Program::Uniform BACKDROP_KERNEL_RADIUS("uKernelRadius");
const gl::Program::Uniform BACKDROP_UVMAP("uvmap");

}

OpenGLBackend::TransientBuffer::TransientBuffer(GLenum target, size_t size) : mTarget(target), mSize(size) {
    glGenBuffers(1, &mHandle);
    bind();
    glBufferData(target, size, nullptr, GL_STREAM_DRAW);
}

OpenGLBackend::TransientBuffer::~TransientBuffer() {
    if (mHandle) glDeleteBuffers(1, &mHandle);
}

void OpenGLBackend::TransientBuffer::bind() {
    glBindBuffer(mTarget, mHandle);
}

void OpenGLBackend::TransientBuffer::orphan() {
    bind();
    glBufferData(mTarget, mSize, nullptr, GL_STREAM_DRAW);
    mOffset = 0;
}

size_t OpenGLBackend::TransientBuffer::upload(const void* data, size_t size) {
    if (mOffset + size > mSize) orphan();
    size_t res = mOffset;
    bind();
    glBufferSubData(mTarget, mOffset, size, data);
    mOffset += (size + 15) & ~15; // align to 16 bytes
    return res;
}

_unique<IOffscreenRenderPass> OpenGLBackend::beginOffscreen(const _<ITexture>& renderTarget) {
    renderTarget->setOrigin(TextureOrigin::TOP_LEFT);
    return std::make_unique<CommonOffscreenRenderPass>(*this, renderTarget);
}

void OpenGLBackend::endOffscreen(_unique<IOffscreenRenderPass> pass) {
    if (auto c = dynamic_cast<CommonOffscreenRenderPass*>(pass.get())) {
        c->displayList.optimize();
        c->displayList.draw(*this, c->target);
    }
}

glm::vec4 toVec4(const ARect<float>& rect) {
    return { rect.p1.x, rect.p1.y, rect.size().x, rect.size().y };
}

ARect<float> toRect(const glm::vec4& rect) {
    return ARect<float>::fromTopLeftPositionAndSize({ rect.x, rect.y }, { rect.z, rect.w });
}

glm::vec4 toGlRect(const glm::vec4& rect, glm::uvec2 viewportSize) {
    return { rect.x, (float)viewportSize.y - rect.y - rect.w, rect.z, rect.w };
}

void hashFloat(std::size_t& seed, float value) {
    aui::hash_combine(seed, std::bit_cast<uint32_t>(value));
}

void hashVec4(std::size_t& seed, const glm::vec4& value) {
    hashFloat(seed, value.x);
    hashFloat(seed, value.y);
    hashFloat(seed, value.z);
    hashFloat(seed, value.w);
}

std::size_t roundedRectMaskCacheKey(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) {
    std::size_t seed = 0;
    hashVec4(seed, toVec4(rect));
    hashFloat(seed, radius);
    aui::hash_combine(seed, inverted);
    hashVec4(seed, toVec4(bounds));
    return seed;
}

std::size_t mergedMaskCacheKey(ITexture* mask1, const glm::vec4& mask1Rect, ITexture* mask2, const glm::vec4& mask2Rect) {
    std::size_t seed = 0;
    aui::hash_combine(seed, reinterpret_cast<std::uintptr_t>(mask1));
    hashVec4(seed, mask1Rect);
    aui::hash_combine(seed, reinterpret_cast<std::uintptr_t>(mask2));
    hashVec4(seed, mask2Rect);
    return seed;
}

OpenGLBackend::OpenGLBackend() :
    mVertexBuffer(GL_ARRAY_BUFFER, 2 * 1024 * 1024),
    mIndexBuffer(GL_ELEMENT_ARRAY_BUFFER, 1024 * 1024),
    mFontCache(AFontManager::inst().createCache(this))
{
    auto readShader = [](const AString& name) {
        auto buffer = AByteBuffer::fromStream(ABuiltinFiles::open(name + ".glsl"));
        return std::string(buffer.begin(), buffer.end());
    };

    auto useShader = [&](AOptional<gl::Program>& out, const AString& vertex, const AString& fragment, std::initializer_list<std::pair<int, const char*>> attrs) {
        out.emplace();
        out->loadVertexShader(readShader(vertex));
        out->loadFragmentShader(readShader(fragment));

        for (auto [index, name] : attrs) {
            out->bindAttribute(index, name);
        }
        out->compile();
    };

    useShader(mSolidShader, "basic.vsh", "rect_solid.fsh", {{0, "pos"}, {1, "color"}});
    useShader(mBoxShadowShader, "basic_uv.vsh", "shadow.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mBoxShadowInnerShader, "basic_uv.vsh", "shadow_inner.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}, {3, "outerSize"}});
    useShader(mRoundedSolidShader, "basic_uv.vsh", "rect_solid_rounded.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}, {3, "outerSize"}});
    useShader(mRoundedSolidShaderBorder, "basic_uv.vsh", "border_rounded.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}, {3, "outerSize"}, {4, "innerSize"}, {5, "outerToInner"}});
    useShader(mGradientShader, "basic_uv.vsh", "rect_gradient.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mRoundedGradientShader, "basic_uv.vsh", "rect_gradient_rounded.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}, {3, "outerSize"}, {6, "color1"}, {7, "color2"}});
    useShader(mTexturedShader, "basic_uv.vsh", "rect_textured.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mRoundedTexturedShader, "basic_uv.vsh", "rect_textured_rounded.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}, {3, "outerSize"}});
    useShader(mSquareSectorShader, "basic_uv.vsh", "square_sector.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mSymbolShader, "symbol.vsh", "symbol.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mSymbolShaderSubPixel, "symbol.vsh", "symbol_sub.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mLineSolidDashedShader, "basic_uv.vsh", "line_solid_dashed.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mUnblendShader, "basic_uv.vsh", "rect_unblend.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mMergeMasksShader, "merge_masks.vsh", "merge_masks.fsh", {{0, "pos"}, {1, "uv"}});
    useShader(mBackdropBlurShader, "basic_uv.vsh", "backdrop_blur.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});
    useShader(mBackdropLiquidShader, "basic_uv.vsh", "backdrop_liquid.fsh", {{0, "pos"}, {1, "uv"}, {2, "color"}});

    mEmptyMask = createTexture({1, 1}, APixelFormat::R8_UNORM);
    AImage emptyImg({1, 1}, APixelFormat::R8_UNORM);
    std::memset(emptyImg.modifiableBuffer().data(), 0, emptyImg.modifiableBuffer().getSize());
    static_cast<OpenGLTexture2D*>(mEmptyMask.get())->upload(emptyImg);

    mBatchVao.bind();
    mVertexBuffer.bind();
    mIndexBuffer.bind();

    AImage whiteImg(glm::uvec2(1, 1), APixelFormat::R8G8B8A8_UNORM);
    whiteImg.set({0, 0}, AColor::WHITE);
    mWhiteTexture.tex2D(whiteImg);
    mWhiteTexture.setupNearest();
}


void OpenGLTexture2D::upload(AImageView image) {
    mTexture.tex2D(image);
    mFormat = image.format();
}

OpenGLTexture2D::~OpenGLTexture2D() {
    if (auto r = mRenderer.lock()) {
        r->onTextureDestroyed(this);
    }
}

_<ITexture> OpenGLBackend::createTexture(glm::u32vec2 size, APixelFormat format, TextureFilter filter) {
    auto t = _new<OpenGLTexture2D>(weak_from_this(), size, format);
    if (filter == TextureFilter::NEAREST) {
        t->texture().setupNearest();
    } else {
        t->texture().setupLinear();
    }
    return t;
}

void OpenGLBackend::setBlending(const APaint& paint) {
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
            glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case Blending::CLEAR:
            glBlendFuncSeparate(GL_ZERO, GL_ZERO, GL_ZERO, GL_ZERO);
            break;
    }
}

_<ITexture> OpenGLBackend::createFramebufferWrapper(uint32_t handle, glm::uvec2 size) {
    mViewportSize = size;
    mProjectionMatrix = glm::ortho(0.f, (float)mViewportSize.x, (float)mViewportSize.y, 0.f, -1.f, 1.f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, (GLsizei)size.x, (GLsizei)size.y);
    return _new<OpenGLFramebufferTexture>(handle, size);
}

void OpenGLBackend::onTextureDestroyed(ITexture* texture) {
    auto glTexture = dynamic_cast<OpenGLTexture2D*>(texture);
    if (!glTexture) {
        return;
    }
    for (auto it = mFboCache.begin(); it != mFboCache.end();) {
        if (it->first.textureHandle == glTexture->texture().getHandle()) {
            mFboCacheList.erase(it->second);
            it = mFboCache.erase(it);
        } else {
            ++it;
        }
    }
}

std::size_t OpenGLBackend::FboCacheKeyHash::operator()(const FboCacheKey& value) const noexcept {
    std::size_t seed = value.textureHandle;
    seed ^= std::size_t(value.textureTarget) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::size_t(value.attachment) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

void OpenGLBackend::trimFboCache() {
    while (mFboCache.size() > MAX_FBO_CACHE_SIZE) {
        auto last = std::prev(mFboCacheList.end());
        mFboCache.erase(last->key);
        mFboCacheList.pop_back();
    }
}

gl::Framebuffer& OpenGLBackend::getFbo(const _<OpenGLTexture2D>& texture) {
    FboCacheKey key {
        .textureHandle = texture->texture().getHandle(),
        .textureTarget = GL_TEXTURE_2D,
        .attachment = GL_COLOR_ATTACHMENT0,
    };
    auto it = mFboCache.find(key);
    if (it == mFboCache.end()) {
        mFboCacheList.push_front(FboCacheEntry { .key = key, .framebuffer = gl::Framebuffer() });
        auto entryIt = mFboCacheList.begin();
        entryIt->framebuffer.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->texture().getHandle(), 0);
        auto [newIt, inserted] = mFboCache.emplace(key, entryIt);
        trimFboCache();
        return entryIt->framebuffer;
    }
    mFboCacheList.splice(mFboCacheList.begin(), mFboCacheList, it->second);
    it->second = mFboCacheList.begin();
    return it->second->framebuffer;
}

void OpenGLBackend::setRenderTarget(const _<ITexture>& texture, glm::uvec2 size) {
    mCurrentRenderTarget = texture;
    mViewportSize = size;
    if (auto glTexture = dynamic_cast<OpenGLTexture2D*>(texture.get())) {
        auto& fbo = getFbo(_cast<OpenGLTexture2D>(texture));
        fbo.bind();
    } else if (auto fbTexture = dynamic_cast<OpenGLFramebufferTexture*>(texture.get())) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbTexture->handle());
    } else {
        gl::Framebuffer::unbind();
    }
    mProjectionMatrix = glm::ortho(0.f, static_cast<float>(mViewportSize.x), static_cast<float>(mViewportSize.y), 0.f, -1.f, 1.f);
    glViewport(0, 0, static_cast<GLsizei>(mViewportSize.x), static_cast<GLsizei>(mViewportSize.y));
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, static_cast<GLsizei>(mViewportSize.x), static_cast<GLsizei>(mViewportSize.y));
}

void OpenGLBackend::setClipRect(const ARect<float>& rect) {
    float x1 = std::max(0.f, rect.p1.x);
    float y1 = std::max(0.f, rect.p1.y);
    float x2 = std::min(static_cast<float>(mViewportSize.x), rect.p2.x);
    float y2 = std::min(static_cast<float>(mViewportSize.y), rect.p2.y);

    int ix1 = static_cast<int>(std::floor(x1));
    int iy1 = static_cast<int>(std::floor(static_cast<float>(mViewportSize.y) - y2));
    int iw = static_cast<int>(std::ceil(x2 - x1));
    int ih = static_cast<int>(std::ceil(y2 - y1));

    glScissor(ix1, iy1, std::max(0, iw), std::max(0, ih));
}

void OpenGLBackend::clear(const AColor& color) {
    glViewport(0, 0, static_cast<GLsizei>(mViewportSize.x), static_cast<GLsizei>(mViewportSize.y));
    glDisable(GL_SCISSOR_TEST);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLBackend::beginRenderPass(const _<ITexture>& target) {
    // Wait, GLDebugGroupLocal is RAII. I should use glPushDebugGroup directly here.
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
    if (glPushDebugGroup) {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Render Pass");
    }
#endif
}

void OpenGLBackend::endRenderPass() {
#if defined(GL_KHR_debug) || defined(GL_VERSION_4_3)
    if (glPopDebugGroup) {
        glPopDebugGroup();
    }
#endif
}

void OpenGLBackend::flush() {
    glFlush();
}

glm::mat4 OpenGLBackend::getProjectionMatrix() const {
    return mProjectionMatrix;
}

bool OpenGLBackend::loadGL(GLLoadProc load_proc, bool es) {
    gladLoadGLLoader(load_proc);
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

uint32_t OpenGLBackend::getDefaultFb() const noexcept { return 0; }

void OpenGLBackend::FramebufferBackToPool::operator()(FramebufferWithTextureRT* framebuffer) const {}

void OpenGLBackend::setupMask(gl::Program& shader) {
    shader.set(aui::ShaderUniforms::WINDOW_SIZE, glm::vec2(mViewportSize));
    if (mMask) {
        shader.set(aui::ShaderUniforms::MASK, 1);
        shader.set(aui::ShaderUniforms::USE_MASK, true);
        shader.set(aui::ShaderUniforms::MASK_RECT, toGlRect(mMaskRect, mViewportSize));
        bindTexture(mMask.get(), 1);
    } else {
        shader.set(aui::ShaderUniforms::MASK, 1);
        shader.set(aui::ShaderUniforms::USE_MASK, false);
        mWhiteTexture.bind(1);
    }
    gl::State::activeTexture(0);
}

void OpenGLBackend::setMask(const _<ITexture>& mask, const glm::vec4& maskRect) {
    mMask = mask;
    mMaskRect = maskRect;
}

_<ITexture> OpenGLBackend::createRectMask(const ARect<float>& rect, bool inverted, const ARect<float>& bounds) {
    GLDebugGroupLocal debugGroup("createRectMask");

    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(bounds.size().x)), std::max(1u, (unsigned)std::ceil(bounds.size().y)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM, TextureFilter::NEAREST);
    auto glDestTexture = _cast<OpenGLTexture2D>(destTexture);
    glDestTexture->texture().setupClampToEdge();

    gl::Framebuffer& framebuffer = getFbo(glDestTexture);

    GLint prevFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
    glm::uvec2 prevViewportSize = mViewportSize;
    glm::mat4 prevProjectionMatrix = mProjectionMatrix;
    bool prevIsRenderingToMask = mIsRenderingToMask;

    framebuffer.bind();
    glViewport(0, 0, size.x, size.y);
    mViewportSize = size;
    mProjectionMatrix = glm::mat4(1.0f);
    mIsRenderingToMask = true;

    glClearColor(inverted ? 1.f : 0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto intersection = rect.intersect(bounds);
    if (intersection.p1.x < intersection.p2.x && intersection.p1.y < intersection.p2.y) {
        glEnable(GL_SCISSOR_TEST);
        glScissor((GLint)(intersection.p1.x - bounds.p1.x),
                  (GLint)(size.y - (intersection.p2.y - bounds.p1.y)),
                  (GLsizei)std::ceil(intersection.size().x),
                  (GLsizei)std::ceil(intersection.size().y));
        glClearColor(inverted ? 0.f : 1.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);
    }

    if (prevFramebuffer != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFramebuffer));
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, getDefaultFb());
    }
    glViewport(0, 0, prevViewportSize.x, prevViewportSize.y);
    mViewportSize = prevViewportSize;
    mProjectionMatrix = prevProjectionMatrix;
    mIsRenderingToMask = prevIsRenderingToMask;

    return destTexture;
}

_<ITexture> OpenGLBackend::createRoundedRectMask(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) {
    const auto cacheKey = roundedRectMaskCacheKey(rect, radius, inverted, bounds);
    if (auto it = mRoundedRectMaskCache.find(cacheKey); it != mRoundedRectMaskCache.end()) {
        if (auto texture = it->second.texture.lock()) {
            return texture;
        }
        mRoundedRectMaskCache.erase(it);
    }

    GLDebugGroupLocal debugGroup("createRoundedRectMask");

    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(bounds.size().x)), std::max(1u, (unsigned)std::ceil(bounds.size().y)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM, TextureFilter::NEAREST);
    auto glDestTexture = _cast<OpenGLTexture2D>(destTexture);
    glDestTexture->texture().setupClampToEdge();

    auto pass = beginOffscreen(destTexture);
    auto ctx = pass->context();
    ctx.canvas.clear(inverted ? AColor::WHITE : AColor::TRANSPARENT_BLACK);
    ctx.canvas.setTransformForced(glm::mat4(1.0f));
    ctx.canvas.roundedRectangle(APaint{ASolidBrush{inverted ? AColor::BLACK : AColor::WHITE}}, rect.min() - bounds.min(), rect.size(), radius);
    endOffscreen(std::move(pass));

    mRoundedRectMaskCache.emplace(cacheKey, RoundedRectMaskCacheEntry { .texture = destTexture });
    return destTexture;
}

IRendererBackend::AMergedMask OpenGLBackend::mergeMasks(const _<ITexture>& mask1, const glm::vec4& mask1Rect,
                                                         const _<ITexture>& mask2, const glm::vec4& mask2Rect) {
    float x = std::max(mask1Rect.x, mask2Rect.x);
    float y = std::max(mask1Rect.y, mask2Rect.y);
    float w = std::min(mask1Rect.x + mask1Rect.z, mask2Rect.x + mask2Rect.z) - x;
    float h = std::min(mask1Rect.y + mask1Rect.w, mask2Rect.y + mask2Rect.w) - y;

    if (w <= 0.f || h <= 0.f || !mask1 || !mask2) {
        return { mEmptyMask, glm::vec4(0.f) };
    }

    const auto cacheKey = mergedMaskCacheKey(mask1.get(), mask1Rect, mask2.get(), mask2Rect);
    if (auto it = mMergedMaskCache.find(cacheKey); it != mMergedMaskCache.end()) {
        if (auto texture = it->second.texture.lock()) {
            return { texture, it->second.rect };
        }
        mMergedMaskCache.erase(it);
    }

    GLDebugGroupLocal debugGroup("mergeMasks");

    GLint prevFramebuffer = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
    glm::uvec2 prevViewportSize = mViewportSize;
    glm::mat4 prevProjectionMatrix = mProjectionMatrix;
    bool prevIsRenderingToMask = mIsRenderingToMask;

    glm::u32vec2 size(std::max(1u, (unsigned)std::ceil(w)), std::max(1u, (unsigned)std::ceil(h)));
    auto destTexture = createTexture(size, APixelFormat::R8_UNORM, TextureFilter::NEAREST);
    auto glDestTexture = _cast<OpenGLTexture2D>(destTexture);
    glDestTexture->texture().setupClampToEdge();

    gl::Framebuffer& framebuffer = getFbo(glDestTexture);

    GLboolean prevBlend = glIsEnabled(GL_BLEND);

    framebuffer.bind();
    glViewport(0, 0, size.x, size.y);
    mViewportSize = size;
    mProjectionMatrix = glm::mat4(1.0f);
    mIsRenderingToMask = true;

    glDisable(GL_BLEND);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    mMergeMasksShader->use();

    glm::vec4 glMask1Rect = toGlRect(mask1Rect, prevViewportSize);
    glm::vec4 glMask2Rect = toGlRect(mask2Rect, prevViewportSize);
    glm::vec4 glDestRect = toGlRect(glm::vec4(x, y, (float)size.x, (float)size.y), prevViewportSize);

    static gl::Program::Uniform u_mask1("u_mask1");
    static gl::Program::Uniform u_mask2("u_mask2");
    static gl::Program::Uniform u_mask1Rect("u_mask1Rect");
    static gl::Program::Uniform u_mask2Rect("u_mask2Rect");
    static gl::Program::Uniform u_destRect("u_destRect");
    static gl::Program::Uniform u_transform("transform");

    mMergeMasksShader->set(u_mask1, 0);
    mMergeMasksShader->set(u_mask2, 1);
    mMergeMasksShader->set(u_mask1Rect, glMask1Rect);
    mMergeMasksShader->set(u_mask2Rect, glMask2Rect);
    mMergeMasksShader->set(u_destRect, glDestRect);
    mMergeMasksShader->set(u_transform, glm::mat4(1.0f));

    bindTexture(mask1.get(), 0);
    bindTexture(mask2.get(), 1);

    AVector<VertexBasicUv> vertices;
    vertices << VertexBasicUv{{-1.f, -1.f}, {0.f, 0.f}, glm::vec4(1.f)};
    vertices << VertexBasicUv{{ 1.f, -1.f}, {1.f, 0.f}, glm::vec4(1.f)};
    vertices << VertexBasicUv{{-1.f,  1.f}, {0.f, 1.f}, glm::vec4(1.f)};
    vertices << VertexBasicUv{{ 1.f,  1.f}, {1.f, 1.f}, glm::vec4(1.f)};

    AVector<GLuint> indices;
    indices << 0 << 1 << 2 << 2 << 1 << 3;

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);

    gl::State::activeTexture(1);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);
    gl::State::activeTexture(0);
    gl::State::bindTexture(GL_TEXTURE_2D, 0);

    if (prevFramebuffer != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFramebuffer));
    } else {
        gl::Framebuffer::unbind();
    }

    glViewport(0, 0, prevViewportSize.x, prevViewportSize.y);
    mViewportSize = prevViewportSize;
    mProjectionMatrix = prevProjectionMatrix;
    mIsRenderingToMask = prevIsRenderingToMask;

    if (prevBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);

    AMergedMask merged { destTexture, glm::vec4(x, y, (float)size.x, (float)size.y) };
    mMergedMaskCache.emplace(cacheKey, MergedMaskCacheEntry { .texture = destTexture, .rect = merged.rect });
    return merged;
}

void OpenGLBackend::solidRectangles(const ADrawList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRectangles");
    setBlending(paint);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mSolidShader);

    AVector<VertexBasic> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = maskColor(inst.color).premultiply();
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

void OpenGLBackend::gradientRectangles(const ADrawList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRectangles");
    setBlending(paint);
    mGradientShader->use();
    mGradientShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mGradientShader);

    ALinearGradientBrush brush {
        v.colors,
        v.rotation
    };
    aui::render::brush::gradient::Helper helper(brush);
    mGradientShader->set(aui::ShaderUniforms::GRADIENT_MAT_UV, helper.matrix);

    glm::vec4 c1 = v.colors.size() > 0 ? maskColor(v.colors[0].color).premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? maskColor(v.colors[1].color).premultiply() : (v.colors.size() > 0 ? maskColor(v.colors[0].color).premultiply() : glm::vec4(0.f));
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
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexBasicUv{rectVertices[0], {0.f, 0.f}, color};
        vertices << VertexBasicUv{rectVertices[1], {1.f, 0.f}, color};
        vertices << VertexBasicUv{rectVertices[2], {0.f, 1.f}, color};
        vertices << VertexBasicUv{rectVertices[3], {1.f, 1.f}, color};

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

void OpenGLBackend::texturedRectangles(const ADrawList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty() || !v.texture) return;
    GLDebugGroupLocal debugGroup("texturedRectangles");
    setBlending(paint);
    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    mTexturedShader->set(aui::ShaderUniforms::ALBEDO, 0);
    mTexturedShader->set(aui::ShaderUniforms::PREMULTIPLIED, v.premultiplied);
    setupMask(*mTexturedShader);
    bindTexture(v.texture.get());

    glm::vec2 uv1 = v.uv1;
    glm::vec2 uv2 = v.uv2;

    AVector<VertexBasicUv> vertices;
    AVector<GLuint> indices;
    vertices.reserve(v.instances.size() * 4);
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        const auto& inst = v.instances[i];
        GLuint offset = static_cast<GLuint>(i * 4);
        auto rectVertices = getVerticesForRect(inst.position, inst.size);
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexBasicUv{rectVertices[0], {uv1.x, uv1.y}, color};
        vertices << VertexBasicUv{rectVertices[1], {uv2.x, uv1.y}, color};
        vertices << VertexBasicUv{rectVertices[2], {uv1.x, uv2.y}, color};
        vertices << VertexBasicUv{rectVertices[3], {uv2.x, uv2.y}, color};

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

void OpenGLBackend::solidRoundedRectangles(const ADrawList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("solidRoundedRectangles");
    setBlending(paint);
    mRoundedSolidShader->use();
    mRoundedSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mRoundedSolidShader);

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
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexRounded{rectVertices[0], {0.f, 0.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[1], {1.f, 0.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[2], {0.f, 1.f}, color, outerSize};
        vertices << VertexRounded{rectVertices[3], {1.f, 1.f}, color, outerSize};

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

void OpenGLBackend::gradientRoundedRectangles(const ADrawList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("gradientRoundedRectangles");
    setBlending(paint);
    mRoundedGradientShader->use();
    mRoundedGradientShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mRoundedGradientShader);

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

    glm::vec4 c1 = v.colors.size() > 0 ? maskColor(v.colors[0].color).premultiply() : glm::vec4(0.f);
    glm::vec4 c2 = v.colors.size() > 1 ? maskColor(v.colors[1].color).premultiply() : (v.colors.size() > 0 ? maskColor(v.colors[0].color).premultiply() : glm::vec4(0.f));

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
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexRoundedGradient{rectVertices[0], {0.f, 0.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[1], {1.f, 0.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[2], {0.f, 1.f}, color, outerSize, c1, c2};
        vertices << VertexRoundedGradient{rectVertices[3], {1.f, 1.f}, color, outerSize, c1, c2};

        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, outerSize)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color1)));
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRoundedGradient), (void*)(vOffset + offsetof(VertexRoundedGradient, color2)));

    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLBackend::texturedRoundedRectangles(const ADrawList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty() || !v.texture) return;
    GLDebugGroupLocal debugGroup("texturedRoundedRectangles");
    setBlending(paint);
    mRoundedTexturedShader->use();
    mRoundedTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    mRoundedTexturedShader->set(aui::ShaderUniforms::ALBEDO, 0);
    mRoundedTexturedShader->set(aui::ShaderUniforms::PREMULTIPLIED, v.premultiplied);
    setupMask(*mRoundedTexturedShader);
    bindTexture(v.texture.get());

    glm::vec2 uv1 = v.uv1;
    glm::vec2 uv2 = v.uv2;

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
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexRounded{rectVertices[0], {uv1.x, uv1.y}, color, outerSize};
        vertices << VertexRounded{rectVertices[1], {uv2.x, uv1.y}, color, outerSize};
        vertices << VertexRounded{rectVertices[2], {uv1.x, uv2.y}, color, outerSize};
        vertices << VertexRounded{rectVertices[3], {uv2.x, uv2.y}, color, outerSize};

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

void OpenGLBackend::rectangleBorders(const ADrawList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("rectangleBorders");
    setBlending(paint);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mSolidShader);

    AVector<VertexBasic> vertices;
    AVector<GLuint> indices;

    for (const auto& inst : v.instances) {
        glm::vec4 color = maskColor(inst.color).premultiply();

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

void OpenGLBackend::roundedRectangleBorders(const ADrawList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty()) return;
    GLDebugGroupLocal debugGroup("roundedRectangleBorders");
    setBlending(paint);
    mRoundedSolidShaderBorder->use();
    mRoundedSolidShaderBorder->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mRoundedSolidShaderBorder);

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
        glm::vec4 color = maskColor(inst.color).premultiply();

        vertices << VertexRoundedBorder{rectVertices[0], {0.f, 0.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[1], {1.f, 0.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[2], {0.f, 1.f}, color, outerSize, innerSize, outerToInner};
        vertices << VertexRoundedBorder{rectVertices[3], {1.f, 1.f}, color, outerSize, innerSize, outerToInner};

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

void OpenGLBackend::boxShadow(const ADrawList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("boxShadow");
    setBlending(paint);
    mBoxShadowShader->use();
    mBoxShadowShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mBoxShadowShader);

    float sigma = v.blurRadius / 2.f;
    float padding = v.blurRadius * 2.f;
    glm::vec2 pos = v.position - padding;
    glm::vec2 size = v.size + padding * 2.f;
    auto rectVertices = getVerticesForRect(pos, size);
    glm::vec4 color = maskColor(v.color).premultiply();

    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, v.position);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, v.position + v.size);
    mBoxShadowShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, sigma);

    VertexBasic vertices[4];
    for (int i = 0; i < 4; ++i) vertices[i] = {rectVertices[i], color};
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLBackend::boxShadowInner(const ADrawList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("boxShadowInner");
    setBlending(paint);
    mBoxShadowInnerShader->use();
    mBoxShadowInnerShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mBoxShadowInnerShader);

    float sigma = v.blurRadius / 2.f;
    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = maskColor(v.color).premultiply();

    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_LOWER, v.position + v.offset + v.spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_UPPER, v.position + v.size + v.offset - v.spreadRadius);
    mBoxShadowInnerShader->set(aui::ShaderUniforms::SL_UNIFORM_SIGMA, sigma);

    glm::vec2 outerSize = glm::vec2(2.f * v.borderRadius) / v.size;

    VertexRounded vertices[4] = {
        {rectVertices[0], {0.f, 0.f}, color, outerSize},
        {rectVertices[1], {1.f, 0.f}, color, outerSize},
        {rectVertices[2], {0.f, 1.f}, color, outerSize},
        {rectVertices[3], {1.f, 1.f}, color, outerSize},
    };
    GLuint indices[6] = {0, 1, 2, 2, 1, 3};

    size_t vOffset = mVertexBuffer.upload(vertices, sizeof(vertices));
    size_t iOffset = mIndexBuffer.upload(indices, sizeof(indices));

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, pos)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, uv)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, color)));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexRounded), (void*)(vOffset + offsetof(VertexRounded, outerSize)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)iOffset);
}

void OpenGLBackend::glyphs(const ADrawList::Glyphs& v, const glm::mat4& transform, const APaint& paint) {
    if (v.instances.empty() || !v.texture) return;
    GLDebugGroupLocal debugGroup(v.isSubpixel ? "glyphsSubpixel" : "glyphsGrayscale");
    setBlending(paint);

    if (v.isSubpixel) {
        mSymbolShaderSubPixel->use();
        mSymbolShaderSubPixel->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
        mSymbolShaderSubPixel->set(aui::ShaderUniforms::ALBEDO, 0);
        setupMask(*mSymbolShaderSubPixel);
    } else {
        mSymbolShader->use();
        mSymbolShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
        mSymbolShader->set(aui::ShaderUniforms::ALBEDO, 0);
        setupMask(*mSymbolShader);
    }
    bindTexture(v.texture.get());

    AVector<GLuint> indices;
    indices.reserve(v.instances.size() * 6);
    for (size_t i = 0; i < v.instances.size(); ++i) {
        GLuint offset = static_cast<GLuint>(i * 4);
        indices << offset + 0 << offset + 1 << offset + 2 << offset + 2 << offset + 1 << offset + 3;
    }
    size_t iOffset = mIndexBuffer.upload(indices.data(), indices.sizeInBytes());

    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    if (v.isSubpixel) {
        AVector<VertexBasicUv> pass1Vertices;
        AVector<VertexBasicUv> pass2Vertices;
        pass1Vertices.reserve(v.instances.size() * 4);
        pass2Vertices.reserve(v.instances.size() * 4);

        for (size_t i = 0; i < v.instances.size(); ++i) {
            const auto& inst = v.instances[i];
            auto rectVertices = getVerticesForRect(inst.position, inst.size);
            glm::vec4 premulColor = maskColor(inst.color).premultiply();
            glm::vec4 pass1Color(1.f, 1.f, 1.f, premulColor.a);

            pass1Vertices << VertexBasicUv{rectVertices[0], {inst.u1.x, inst.u1.y}, pass1Color};
            pass1Vertices << VertexBasicUv{rectVertices[1], {inst.u2.x, inst.u1.y}, pass1Color};
            pass1Vertices << VertexBasicUv{rectVertices[2], {inst.u1.x, inst.u2.y}, pass1Color};
            pass1Vertices << VertexBasicUv{rectVertices[3], {inst.u2.x, inst.u2.y}, pass1Color};

            pass2Vertices << VertexBasicUv{rectVertices[0], {inst.u1.x, inst.u1.y}, premulColor};
            pass2Vertices << VertexBasicUv{rectVertices[1], {inst.u2.x, inst.u1.y}, premulColor};
            pass2Vertices << VertexBasicUv{rectVertices[2], {inst.u1.x, inst.u2.y}, premulColor};
            pass2Vertices << VertexBasicUv{rectVertices[3], {inst.u2.x, inst.u2.y}, premulColor};
        }

        size_t v1Offset = mVertexBuffer.upload(pass1Vertices.data(), pass1Vertices.sizeInBytes());
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v1Offset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v1Offset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v1Offset + offsetof(VertexBasicUv, color)));
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);

        size_t v2Offset = mVertexBuffer.upload(pass2Vertices.data(), pass2Vertices.sizeInBytes());
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v2Offset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v2Offset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(v2Offset + offsetof(VertexBasicUv, color)));
        glBlendFunc(GL_ONE, GL_ONE);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);

        glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        AVector<VertexBasicUv> vertices;
        vertices.reserve(v.instances.size() * 4);
        for (size_t i = 0; i < v.instances.size(); ++i) {
            const auto& inst = v.instances[i];
            auto rectVertices = getVerticesForRect(inst.position, inst.size);
            glm::vec4 color = maskColor(inst.color).premultiply();

            vertices << VertexBasicUv{rectVertices[0], {inst.u1.x, inst.u1.y}, color};
            vertices << VertexBasicUv{rectVertices[1], {inst.u2.x, inst.u1.y}, color};
            vertices << VertexBasicUv{rectVertices[2], {inst.u1.x, inst.u2.y}, color};
            vertices << VertexBasicUv{rectVertices[3], {inst.u2.x, inst.u2.y}, color};
        }

        size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, pos)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, uv)));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasicUv), (void*)(vOffset + offsetof(VertexBasicUv, color)));
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, (void*)iOffset);
    }
}

_<IRenderer::IMultiStringCanvas> OpenGLBackend::newMultiStringCanvas(const AFontStyle& style) {
    auto entryData = aui::getFontEntryData(style, mFontCache);
    return _new<aui::MultiStringCanvas>(*this, entryData, style);
}

_<IRenderer::IPrerenderedString> OpenGLBackend::prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) {
    if (text.empty()) return nullptr;
    auto c = newMultiStringCanvas(fs);
    c->addString(position, text);
    return c->finalize();
}

bool OpenGLBackend::setupLineShader(const glm::mat4& transform, const ABorderStyle& style, float widthPx, const APaint& paint) {
    return std::visit(aui::lambda_overloaded{
        [&](const ABorderStyle::Solid&) {
            mSolidShader->use();
            mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
            setupMask(*mSolidShader);
            return false;
        },
        [&](const ABorderStyle::Dashed& dashed) {
            mLineSolidDashedShader->use();
            mLineSolidDashedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
            setupMask(*mLineSolidDashedShader);
            float dashWidth = dashed.dashWidth.valueOr(1.f) * widthPx;
            float sumOfLengths = dashWidth + dashed.spaceBetweenDashes.valueOr(2.f) * widthPx;
            mLineSolidDashedShader->set(aui::ShaderUniforms::DIVIDER, sumOfLengths);
            mLineSolidDashedShader->set(aui::ShaderUniforms::THRESHOLD, dashWidth);
            return true;
        }
    }, style.value());
}

void OpenGLBackend::lines(const ADrawList::Lines& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.size() < 2) return;
    GLDebugGroupLocal debugGroup("lines");
    setBlending(paint);
    float widthPx = v.width.getValuePx();
    glLineWidth(widthPx);
    bool computeDistances = setupLineShader(transform, v.style, widthPx, paint);

    AVector<VertexBasicUv> vertices;
    vertices.reserve(v.points.size());
    glm::vec4 color = maskColor(paint.color).premultiply();

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

void OpenGLBackend::points(const ADrawList::Points& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    GLDebugGroupLocal debugGroup("points");
    setBlending(paint);
    mSolidShader->use();
    mSolidShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mSolidShader);
    glPointSize(v.size.getValuePx());

    AVector<VertexBasic> vertices;
    vertices.reserve(v.points.size());
    glm::vec4 color = maskColor(paint.color).premultiply();
    for (const auto& p : v.points) {
        vertices << VertexBasic{p, color};
    }

    size_t vOffset = mVertexBuffer.upload(vertices.data(), vertices.sizeInBytes());
    mBatchVao.bind();
    glEnableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, pos)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBasic), (void*)(vOffset + offsetof(VertexBasic, color)));

    glDrawArrays(GL_POINTS, 0, (GLsizei)vertices.size());
}

void OpenGLBackend::lines(const ADrawList::LineBatches& v, const glm::mat4& transform, const APaint& paint) {
    if (v.points.empty()) return;
    GLDebugGroupLocal debugGroup("lineBatches");
    setBlending(paint);
    float widthPx = v.width.getValuePx();
    glLineWidth(widthPx);
    bool computeDistances = setupLineShader(transform, v.style, widthPx, paint);

    AVector<VertexBasicUv> vertices;
    vertices.reserve(v.points.size() * 2);
    glm::vec4 color = maskColor(paint.color).premultiply();

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

void OpenGLBackend::squareSector(const ADrawList::SquareSector& v, const glm::mat4& transform, const APaint& paint) {
    GLDebugGroupLocal debugGroup("squareSector");
    setBlending(paint);
    mSquareSectorShader->use();
    mSquareSectorShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix * transform);
    setupMask(*mSquareSectorShader);

    mSquareSectorShader->set(aui::ShaderUniforms::BEGIN, v.begin.radians());
    mSquareSectorShader->set(aui::ShaderUniforms::END, v.end.radians());

    auto rectVertices = getVerticesForRect(v.position, v.size);
    glm::vec4 color = maskColor(paint.color).premultiply();

    VertexBasicUv vertices[4] = {
        {rectVertices[0], {0.f, 0.f}, color},
        {rectVertices[1], {1.f, 0.f}, color},
        {rectVertices[2], {0.f, 1.f}, color},
        {rectVertices[3], {1.f, 1.f}, color},
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

void OpenGLBackend::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {
    if (!glm::all(glm::greaterThan(size, glm::ivec2(0)))) return;
    if (backdrops.empty()) return;
    if (!mCurrentRenderTarget) return;

    auto currentTexture = dynamic_cast<OpenGLTexture2D*>(mCurrentRenderTarget.get());
    auto currentFramebufferTexture = dynamic_cast<OpenGLFramebufferTexture*>(mCurrentRenderTarget.get());
    if (currentFramebufferTexture && currentFramebufferTexture->handle() == 0) {
        return;
    }

    auto bindCurrentRenderTarget = [&] {
        if (currentTexture) {
            getFbo(_cast<OpenGLTexture2D>(mCurrentRenderTarget)).bind();
        } else if (currentFramebufferTexture) {
            glBindFramebuffer(GL_FRAMEBUFFER, currentFramebufferTexture->handle());
        } else {
            gl::Framebuffer::unbind();
        }
        glViewport(0, 0, static_cast<GLsizei>(mViewportSize.x), static_cast<GLsizei>(mViewportSize.y));
    };

    auto bindCurrentRenderTargetForRead = [&] {
        if (currentTexture) {
            getFbo(_cast<OpenGLTexture2D>(mCurrentRenderTarget)).bindForRead();
        } else if (currentFramebufferTexture) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, currentFramebufferTexture->handle());
        }
    };

    auto sourceSize = glm::ivec2(mViewportSize);

    GLDebugGroupLocal debugGroup("backdrops");

    struct AreaOfInterest {
        FramebufferFromPool framebuffer;
        glm::ivec2 size {};
    };

    AOptional<AreaOfInterest> areaOfInterest;

    auto sourceVertices = getVerticesForRect(glm::vec2(position), glm::vec2(size));
    std::array<glm::ivec2, 4> transformedVertices{};
    for (size_t i = 0; i < transformedVertices.size(); ++i) {
        auto ndc = glm::vec2(mProjectionMatrix * glm::vec4(sourceVertices[i], 0, 1));
        transformedVertices[i] = glm::ivec2((ndc + 1.f) / 2.f * glm::vec2(sourceSize));
    }
    auto sourceLower = transformedVertices[0];
    auto sourceUpper = transformedVertices[0];
    for (const auto& v : transformedVertices) {
        sourceLower = glm::min(sourceLower, v);
        sourceUpper = glm::max(sourceUpper, v);
    }

    auto initAreaOfInterest = [&](glm::ivec2 fbSize) {
        auto fb = getFramebufferForMultiPassEffect(fbSize);
        if (auto texture = dynamic_cast<ITexture*>(fb->renderTarget.get())) {
            texture->setOrigin(TextureOrigin::TOP_LEFT);
        }
        bindCurrentRenderTargetForRead();
        fb->framebuffer.bindForWrite();
        glBlitFramebuffer(sourceLower.x, sourceLower.y, sourceUpper.x, sourceUpper.y,
                          0, 0, fbSize.x, fbSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        areaOfInterest = AreaOfInterest{
            .framebuffer = std::move(fb),
            .size = fbSize,
        };
    };

    auto initAreaOfInterestIfEmpty = [&](glm::ivec2 fbSize) {
        if (!areaOfInterest) {
            initAreaOfInterest(fbSize);
        }
    };

    auto bindAreaTexture = [&](const AreaOfInterest& area, uint32_t unit = 0) {
        bindTexture(dynamic_cast<ITexture*>(area.framebuffer->renderTarget.get()), unit);
    };

    auto drawBlurPass = [&](gl::Framebuffer& target, glm::ivec2 targetSize, gl::Framebuffer::IRenderTarget& sourceTexture,
                            glm::vec2 stepSize, glm::vec2 uvMax, unsigned radius) {
        target.bind();
        glViewport(0, 0, targetSize.x, targetSize.y);
        glScissor(0, 0, targetSize.x, targetSize.y);

        mBackdropBlurShader->use();
        mBackdropBlurShader->set(aui::ShaderUniforms::TRANSFORM,
                                 glm::ortho(0.f, float(targetSize.x), float(targetSize.y), 0.f, -1.f, 1.f));
        mBackdropBlurShader->set(aui::ShaderUniforms::COLOR, glm::vec4(1.f));
        mBackdropBlurShader->set(aui::ShaderUniforms::PIXEL_TO_UV, stepSize);
        mBackdropBlurShader->set(aui::ShaderUniforms::M2, uvMax);
        mBackdropBlurShader->set(BACKDROP_KERNEL_RADIUS, int(radius));
        mBackdropBlurShader->setArray(aui::ShaderUniforms::KERNEL, aui::detail::gaussianKernel(radius));
        bindTexture(dynamic_cast<ITexture*>(&sourceTexture));

        auto rectVertices = getVerticesForRect({0.f, 0.f}, glm::vec2(targetSize));
        VertexBasicUv vertices[4] = {
            {rectVertices[0], {0.f, 0.f}, glm::vec4(1.f)},
            {rectVertices[1], {1.f, 0.f}, glm::vec4(1.f)},
            {rectVertices[2], {0.f, 1.f}, glm::vec4(1.f)},
            {rectVertices[3], {1.f, 1.f}, glm::vec4(1.f)},
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
    };

    {
        auto previousBlendEnabled = glIsEnabled(GL_BLEND);
        auto previousScissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
        GLint previousScissorBox[4] = {};
        glGetIntegerv(GL_SCISSOR_BOX, previousScissorBox);

        AUI_DEFER {
            if (previousBlendEnabled) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            if (previousScissorEnabled) {
                glEnable(GL_SCISSOR_TEST);
                glScissor(previousScissorBox[0], previousScissorBox[1], previousScissorBox[2], previousScissorBox[3]);
            } else {
                glDisable(GL_SCISSOR_TEST);
            }
            bindCurrentRenderTarget();
        };

        glDisable(GL_BLEND);

        for (const auto& backdrop : backdrops) {
            std::visit(aui::lambda_overloaded{
                [&](const ass::Backdrop::LiquidFluid&) {
                    AUI_ASSERTX(!areaOfInterest, "LiquidGlass must be the first effect in backdrop list.");
                    if (areaOfInterest) {
                        return;
                    }

                    auto targetSize = sourceSize;
                    auto fb = getFramebufferForMultiPassEffect(targetSize);
                    if (auto texture = dynamic_cast<ITexture*>(fb->renderTarget.get())) {
                        texture->setOrigin(TextureOrigin::TOP_LEFT);
                    }
                    bindCurrentRenderTargetForRead();
                    fb->framebuffer.bindForWrite();
                    glBlitFramebuffer(0, 0, targetSize.x, targetSize.y,
                                      0, 0, targetSize.x, targetSize.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                    areaOfInterest = AreaOfInterest {
                        .framebuffer = std::move(fb),
                        .size = targetSize,
                    };

                    static auto uvMap = [] {
                        auto result = std::make_unique<gl::Texture2D>();
                        result->tex2D(*AImage::fromUrl(":uni/liquid_glass1.png"));
                        return result;
                    }();

                    auto offscreen = getFramebufferForMultiPassEffect(size);
                    if (auto texture = dynamic_cast<ITexture*>(offscreen->renderTarget.get())) {
                        texture->setOrigin(TextureOrigin::TOP_LEFT);
                    }
                    offscreen->framebuffer.bind();
                    glViewport(0, 0, size.x, size.y);
                    glScissor(0, 0, size.x, size.y);

                    bindAreaTexture(*areaOfInterest);
                    uvMap->bind(1);

                    mBackdropLiquidShader->use();
                    mBackdropLiquidShader->set(aui::ShaderUniforms::TRANSFORM,
                                               glm::ortho(0.f, float(size.x), float(size.y), 0.f, -1.f, 1.f));
                    mBackdropLiquidShader->set(aui::ShaderUniforms::UV_SCALE,
                                               glm::vec2(areaOfInterest->size) /
                                                   glm::vec2(areaOfInterest->framebuffer->framebuffer.size()));
                    mBackdropLiquidShader->set(BACKDROP_UVMAP, 1);

                    auto m = mProjectionMatrix;
                    m = glm::translate(m, glm::vec3(0, size.y, 0));
                    m = glm::scale(m, glm::vec3(1, -1, 1));
                    m = glm::translate(m, glm::vec3(position, 0.f));
                    m = glm::scale(m, glm::vec3(size, 1.f));
                    mBackdropLiquidShader->set(aui::ShaderUniforms::M2, m);

                    auto rectVertices = getVerticesForRect({0.f, 0.f}, glm::vec2(size));
                    VertexBasicUv vertices[4] = {
                        {rectVertices[0], {0.f, 0.f}, glm::vec4(1.f)},
                        {rectVertices[1], {1.f, 0.f}, glm::vec4(1.f)},
                        {rectVertices[2], {0.f, 1.f}, glm::vec4(1.f)},
                        {rectVertices[3], {1.f, 1.f}, glm::vec4(1.f)},
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
                    gl::State::activeTexture(1);
                    gl::State::bindTexture(GL_TEXTURE_2D, 0);
                    gl::State::activeTexture(0);

                    areaOfInterest = AreaOfInterest {
                        .framebuffer = std::move(offscreen),
                        .size = size,
                    };
                },
                [&](const ass::Backdrop::GaussianBlurCustom& blur) {
                    auto radius = glm::min(64u, unsigned(glm::max(0, int(blur.radius.getValuePx()))));
                    if (radius <= 1) {
                        return;
                    }

                    AUI_ASSERT(blur.downscale > 0);
                    auto effectSize = glm::max(glm::ivec2(1), size / blur.downscale);

                    initAreaOfInterestIfEmpty(effectSize);

                    auto areaFramebufferSize = areaOfInterest->framebuffer->framebuffer.size();
                    auto stepX = glm::vec2(1.f / float(areaFramebufferSize.x), 0.f);
                    auto uvMaxX = glm::vec2(areaOfInterest->size) / glm::vec2(areaFramebufferSize) - stepX * 0.5f;

                    auto temp = getFramebufferForMultiPassEffect(effectSize);
                    if (auto texture = dynamic_cast<ITexture*>(temp->renderTarget.get())) {
                        texture->setOrigin(TextureOrigin::TOP_LEFT);
                    }

                    drawBlurPass(temp->framebuffer, effectSize, *areaOfInterest->framebuffer->renderTarget,
                                 stepX, uvMaxX, radius);

                    auto tempFramebufferSize = temp->framebuffer.size();
                    auto stepY = glm::vec2(0.f, 1.f / float(tempFramebufferSize.y));
                    auto uvMaxY = glm::vec2(effectSize) / glm::vec2(tempFramebufferSize) - stepY * 0.5f;

                    drawBlurPass(areaOfInterest->framebuffer->framebuffer, effectSize, *temp->renderTarget,
                                 stepY, uvMaxY, radius);

                    areaOfInterest->size = effectSize;
                },
                [&](const auto&) {}
            }, backdrop);
        }
    }

    if (!areaOfInterest) return;

    setBlending(APaint{});
    mTexturedShader->use();
    mTexturedShader->set(aui::ShaderUniforms::TRANSFORM, mProjectionMatrix);
    mTexturedShader->set(aui::ShaderUniforms::COLOR, glm::vec4(1.f));
    mTexturedShader->set(aui::ShaderUniforms::PREMULTIPLIED, true);
    setupMask(*mTexturedShader);
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

OpenGLBackend::FramebufferFromPool OpenGLBackend::getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize) {
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
            auto object = std::make_unique<FramebufferWithTextureRT>();
            object->framebuffer.resize(minRequiredSize);
            object->renderTarget = _cast<gl::Framebuffer::IRenderTarget>(createTexture(minRequiredSize));
            object->framebuffer.attach(object->renderTarget, GL_COLOR_ATTACHMENT0);
            return object.release();
        }(),
        FramebufferBackToPool { this });
}

AImage OpenGLBackend::readback(const _<ITexture>& texture) {
    if (auto glTexture = _cast<OpenGLTexture2D>(texture)) {
        auto& fbo = getFbo(glTexture);
        fbo.bind();
        AImage result(texture->getSize(), texture->getFormat());
        glReadPixels(0, 0, result.width(), result.height(), GL_RGBA, GL_UNSIGNED_BYTE, result.modifiableBuffer().data());
        return result;
    }
    return {};
}
