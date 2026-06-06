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

#pragma once

#include <AUI/GL/gl.h>
#include <AUI/GL/Program.h>
#include <AUI/GL/Framebuffer.h>
#include <AUI/GL/Vao.h>
#include <AUI/GL/Texture2D.h>
#include <AUI/Util/APool.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/ADeque.h>
#include <AUI/Render/FontAtlas.hpp>
#include "AUI/Render/ABorderStyle.h"
#include <AUI/Render/IRendererBackend.h>
#include "AUI/GL/RenderTarget/TextureRenderTarget.h"
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Platform/ASurface.h>
#include <AUI/Render/ACanvas.hpp>
#include <list>
#include <unordered_map>

class OpenGLBackend;

class API_AUI_VIEWS OpenGLTexture2D : public ITexture, public gl::Framebuffer::IRenderTarget {
private:
    gl::Texture2D mTexture;
    APixelFormat mFormat;
    _weak<OpenGLBackend> mRenderer;
public:
    OpenGLTexture2D(_weak<OpenGLBackend> renderer) : mFormat(APixelFormat::R8G8B8A8_UNORM), mRenderer(std::move(renderer)) {}
    OpenGLTexture2D(_weak<OpenGLBackend> renderer, glm::u32vec2 size, APixelFormat format) : mFormat(format), mRenderer(std::move(renderer)) {
        mTexture.tex2D(size, format);
    }
    ~OpenGLTexture2D() override;
    void upload(AImageView image) override;

    glm::u32vec2 getSize() const override {
        return mTexture.getSize();
    }

    [[nodiscard]]
    APixelFormat getFormat() const override {
        return mFormat;
    }

    void bind() { mTexture.bind(); }
    void bind(uint32_t unit) { mTexture.bind(unit); }
    gl::Texture2D& texture() noexcept { return mTexture; }
protected:
    void onFramebufferResize(glm::u32vec2 size) override {
        AUI_ASSERTX(mTexture.getSize() == size, "changing texture size after creation is forbidden");
    }
    void attach(gl::Framebuffer& to, GLenum attachmentType) override {
        to.bind();
        onFramebufferResize(to.size());
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, mTexture.getHandle(), 0);
    }
};

class API_AUI_VIEWS OpenGLBackend final: public IRendererBackend, public std::enable_shared_from_this<OpenGLBackend> {
    friend class OpenGLRenderViewToTexture;
    friend class OpenGLTexture2D;

public:
    typedef void* (*GLLoadProc)(const char* name);

private:
    void onTextureDestroyed(ITexture* texture);
    gl::Framebuffer& getFbo(const _<OpenGLTexture2D>& texture);

    struct FboCacheKey {
        GLuint textureHandle;
        GLenum textureTarget;
        GLenum attachment;

        [[nodiscard]] bool operator==(const FboCacheKey& rhs) const noexcept = default;
    };

    struct FboCacheKeyHash {
        [[nodiscard]] std::size_t operator()(const FboCacheKey& value) const noexcept;
    };

    struct FboCacheEntry {
        FboCacheKey key;
        gl::Framebuffer framebuffer;
    };

    using FboCacheList = std::list<FboCacheEntry>;
    using FboCacheMap = std::unordered_map<FboCacheKey, FboCacheList::iterator, FboCacheKeyHash>;

    static constexpr std::size_t MAX_FBO_CACHE_SIZE = 64;
    void trimFboCache();

    class TransientBuffer {
    public:
        TransientBuffer(GLenum target, size_t size);
        ~TransientBuffer();
        TransientBuffer(const TransientBuffer&) = delete;

        size_t upload(const void* data, size_t size);
        void orphan();
        void bind();
        void bindRange(GLuint index, size_t offset, size_t size);
        [[nodiscard]] GLuint handle() const { return mHandle; }

    private:
        GLenum mTarget;
        GLuint mHandle = 0;
        size_t mSize;
        size_t mOffset = 0;
    };

    struct FramebufferWithTextureRT {
        gl::Framebuffer framebuffer;
        _<gl::Framebuffer::IRenderTarget> renderTarget;
    };

    using OffscreenFramebufferPool = AVector<std::unique_ptr<FramebufferWithTextureRT>>;

    struct FramebufferBackToPool {
        OpenGLBackend* renderer;
        void operator()(FramebufferWithTextureRT* framebuffer) const;
    };

    using FramebufferFromPool = _unique<FramebufferWithTextureRT, FramebufferBackToPool>;

    OffscreenFramebufferPool mFramebuffersForMultiPassEffectsPool;

    FramebufferFromPool getFramebufferForMultiPassEffect(glm::uvec2 size);

    void setupMask(gl::Program& shader);
    bool setupLineShader(const glm::mat4& transform, const ABorderStyle& style, float widthPx, const APaint& paint);

    bool mAllowRenderToTexture = true;
    bool mIsRenderingToMask = false;
    _<ITexture> mMask;
    glm::vec4 mMaskRect = glm::vec4(0.f);

    AOptional<gl::Program> mSolidShader;
    AOptional<gl::Program> mBoxShadowShader;
    AOptional<gl::Program> mBoxShadowInnerShader;
    AOptional<gl::Program> mRoundedSolidShader;
    AOptional<gl::Program> mRoundedSolidShaderBorder;
    AOptional<gl::Program> mGradientShader;
    AOptional<gl::Program> mRoundedGradientShader;
    AOptional<gl::Program> mTexturedShader;
    AOptional<gl::Program> mRoundedTexturedShader;
    AOptional<gl::Program> mUnblendShader;
    AOptional<gl::Program> mSquareSectorShader;
    AOptional<gl::Program> mSymbolShader;
    AOptional<gl::Program> mSymbolShaderSubPixel;
    AOptional<gl::Program> mLineSolidDashedShader;
    AOptional<gl::Program> mMergeMasksShader;
    AOptional<gl::Program> mBackdropBlurShader;
    AOptional<gl::Program> mBackdropLiquidShader;
    _<ITexture> mEmptyMask;
    struct RoundedRectMaskCacheEntry {
        _weak<ITexture> texture;
    };
    struct MergedMaskCacheEntry {
        _weak<ITexture> texture;
        glm::vec4 rect = glm::vec4(0.f);
    };
    std::unordered_map<std::size_t, RoundedRectMaskCacheEntry> mRoundedRectMaskCache;
    std::unordered_map<std::size_t, MergedMaskCacheEntry> mMergedMaskCache;

    gl::Vao mBatchVao;
    gl::Texture2D mGradientTexture;
    gl::Texture2D mWhiteTexture;

    TransientBuffer mVertexBuffer;
    TransientBuffer mIndexBuffer;

    _<aui::AFontCache> mFontCache;

    void setBlending(const APaint& paint);

    glm::uvec2 mViewportSize { 1, 1 };
    glm::mat4 mProjectionMatrix { 1.f };

    static bool mIsES;
    static int mGLSLVersion;

    glm::vec4 maskColor(glm::vec4 color) const noexcept {
        if (mIsRenderingToMask) {
            return glm::vec4(1.f, 0.f, 0.f, color.w);
        }
        return color;
    }
    AColor maskColor(AColor color) const noexcept {
        if (mIsRenderingToMask) {
            return AColor(1.f, 0.f, 0.f, color.a);
        }
        return color;
    }

public:
    OpenGLBackend();

    static bool loadGL(GLLoadProc load_proc, bool es = false);

    [[nodiscard]]
    bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }

    void setAllowRenderToTexture(bool allowRenderToTexture) override { mAllowRenderToTexture = allowRenderToTexture; }

    _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::R8G8B8A8_UNORM, TextureFilter filter = TextureFilter::LINEAR) override;
    _<ITexture> createFramebufferWrapper(uint32_t handle, glm::uvec2 size);

    void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, const APaint& paint) override;
    void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, const APaint& paint) override;
    void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, const APaint& paint) override;
    void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, const APaint& paint) override;
    void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, const APaint& paint) override;
    void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, const APaint& paint) override;
    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) override;
    void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) override;
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) override;
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) override;

    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    glm::mat4 getProjectionMatrix() const override;

    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;

    _unique<IOffscreenRenderPass> beginOffscreen(const _<ITexture>& renderTarget) override;
    void endOffscreen(_unique<IOffscreenRenderPass> pass) override;

    void setRenderTarget(const _<ITexture>& texture, glm::uvec2 size) override;
    void setClipRect(const ARect<float>& rect) override;
    [[nodiscard]]
    _<ITexture> getRenderTarget() const { return mCurrentRenderTarget; }
    [[nodiscard]]
    glm::uvec2 getViewportSize() const override { return mViewportSize; }
    void setRenderMaskMode(bool enabled) override { mIsRenderingToMask = enabled; }
    void clear(const AColor& color) override;
    void beginRenderPass(const _<ITexture>& target) override;
    void endRenderPass() override;
    void flush() override;
    void setMask(const _<ITexture>& mask, const glm::vec4& maskRect = glm::vec4(0.f)) override;

    AMergedMask mergeMasks(const _<ITexture>& mask1, const glm::vec4& mask1Rect,
                                    const _<ITexture>& mask2, const glm::vec4& mask2Rect) override;

    _<ITexture> createRectMask(const ARect<float>& rect, bool inverted, const ARect<float>& bounds) override;
    _<ITexture> createRoundedRectMask(const ARect<float>& rect, float radius, bool inverted, const ARect<float>& bounds) override;

    const _<aui::AFontCache>& getFontCache() override { return mFontCache; }

    uint32_t getDefaultFb() const noexcept;

private:
    FboCacheList mFboCacheList;
    FboCacheMap mFboCache;
    _<ITexture> mCurrentRenderTarget;
};
