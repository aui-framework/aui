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
#include <AUI/Render/IRenderViewToTexture.h>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Platform/ASurface.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/GL/RenderTarget/RenderbufferRenderTarget.h>

class API_AUI_VIEWS OpenGLTexture2D : public ITexture, public gl::Framebuffer::IRenderTarget {
private:
    gl::Texture2D mTexture;
public:
    void upload(AImageView image) override;

    glm::u32vec2 getSize() const override {
        return mTexture.getSize();
    }

    void bind() { mTexture.bind(); }
    void bind(uint32_t unit) { mTexture.bind(unit); }
    gl::Texture2D& texture() noexcept { return mTexture; }
protected:
    void onFramebufferResize(glm::u32vec2 size) override { mTexture.framebufferTex2D(size, gl::Type::UNSIGNED_BYTE); }
    void attach(gl::Framebuffer& to, GLenum attachmentType) override {
        to.bind();
        onFramebufferResize(to.size());
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, mTexture.getHandle(), 0);
    }
};

class API_AUI_VIEWS OpenGLRenderer final: public IRendererBackend {
    friend class OpenGLRenderViewToTexture;

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
        OpenGLRenderer* renderer;
        void operator()(FramebufferWithTextureRT* framebuffer) const;
    };

    using FramebufferFromPool = _unique<FramebufferWithTextureRT, FramebufferBackToPool>;

    std::uint8_t mStencilDepth = 0;

    ASurface* mWindow = nullptr;
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

    gl::Vao mBatchVao;
    gl::Texture2D mGradientTexture;
    gl::Texture2D mWhiteTexture;

    TransientBuffer mVertexBuffer;
    TransientBuffer mIndexBuffer;

    _<aui::AFontCache> mFontCache;

    struct MultiPassEffectData {
        OffscreenFramebufferPool* pool;
    };

    OffscreenFramebufferPool mFramebuffersForMultiPassEffectsPool;

    FramebufferFromPool getFramebufferForMultiPassEffect(glm::uvec2 size);

    glm::uvec2 mViewportSize { 1, 1 };
    glm::mat4 mProjectionMatrix { 1.f };

    static bool mIsES;
    static int mGLSLVersion;

    void setBlending(const APaint& paint);

    void setupMask(gl::Program& shader);
    bool setupLineShader(const glm::mat4& transform, const ABorderStyle& style, float widthPx, const APaint& paint);

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
    OpenGLRenderer();

    typedef void* (*GLLoadProc)(const char* name);
    static bool loadGL(GLLoadProc load_proc, bool es = false);
    static bool loadGL(GLLoadProc load_proc);

    void setWindow(ASurface* window) override;

    [[nodiscard]]
    ASurface* getWindow() const noexcept override { return mWindow; }

    [[nodiscard]]
    bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }

    void setAllowRenderToTexture(bool allowRenderToTexture) override { mAllowRenderToTexture = allowRenderToTexture; }

    _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::RGBA_BYTE, TextureFilter filter = TextureFilter::LINEAR) override;

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

    void beginPaint(glm::uvec2 windowSize);
    void endPaint();

    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    glm::mat4 getProjectionMatrix() const override;

    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;

    void setMask(const _<ITexture>& mask, const glm::vec4& maskRect = glm::vec4(0.f)) override;

    const _<aui::AFontCache>& getFontCache() override { return mFontCache; }

    _unique<IRenderViewToTexture> newRenderViewToTexture(APixelFormat format = APixelFormat::RGBA_BYTE) noexcept;

    uint32_t getDefaultFb() const noexcept;
};

class OpenGLRenderViewToTexture: public IRenderViewToTexture {
public:
    OpenGLRenderViewToTexture(OpenGLRenderer& renderer, APixelFormat format): mRenderer(renderer), mFormat(format) {}

    bool begin(IRenderer& renderer, glm::ivec2 surfaceSize, InvalidArea& invalidArea) override {
        if (!mTexture || mFramebuffer.size() != glm::u32vec2(surfaceSize)) {
            mTexture = _cast<OpenGLTexture2D>(mRenderer.createTexture(surfaceSize, mFormat, TextureFilter::NEAREST));
            mTexture->texture().setupClampToEdge();
            mFramebuffer = gl::Framebuffer();
            mFramebuffer.resize(surfaceSize);
            mFramebuffer.attach(mTexture, GL_COLOR_ATTACHMENT0);
            auto stencil = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::DEPTH24_STENCIL8, gl::Multisampling::DISABLED>>();
            mFramebuffer.attach(stencil, GL_DEPTH_STENCIL_ATTACHMENT);
            invalidArea = InvalidArea::Full{};
        }

        mPrevFramebuffer = gl::Framebuffer::current();
        mPrevViewportSize = mRenderer.mViewportSize;
        mPrevProjectionMatrix = mRenderer.mProjectionMatrix;
        mPrevStencilDepth = mRenderer.mStencilDepth;

        mFramebuffer.bind();
        glViewport(0, 0, surfaceSize.x, surfaceSize.y);
        mRenderer.mViewportSize = glm::uvec2(surfaceSize);
        mRenderer.mProjectionMatrix = glm::ortho(0.f, (float)surfaceSize.x, (float)surfaceSize.y, 0.f, -1.f, 1.f);

        glDisable(GL_STENCIL_TEST);
        mRenderer.mStencilDepth = 0;

        mRenderer.mIsRenderingToMask = (mFormat == APixelFormat::R_BYTE);

        if (glBlendFuncSeparate) {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        return true;
    }

    void end(IRenderer& renderer) override {
        glm::uvec2 viewportSize = mPrevViewportSize;
        if (mPrevFramebuffer) {
            mPrevFramebuffer->bind();
            viewportSize = mPrevFramebuffer->size();
        } else {
            gl::Framebuffer::unbind();
        }
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        mRenderer.mViewportSize = mPrevViewportSize;
        mRenderer.mProjectionMatrix = mPrevProjectionMatrix;
        mRenderer.mStencilDepth = mPrevStencilDepth;
        mRenderer.mIsRenderingToMask = false;
    }

    void draw(ACanvas& canvas) override {
        canvas.rectangle(APaint{ATexturedBrush{
            .texture = mTexture,
            .uv1 = glm::vec2(0.f, 1.f),
            .uv2 = glm::vec2(1.f, 0.f)
        }}, {0, 0}, mTexture->getSize());
    }

    [[nodiscard]]
    _<ITexture> getTexture() const override {
        return mTexture;
    }

    private:
    OpenGLRenderer& mRenderer;
    gl::Framebuffer mFramebuffer;
    _<OpenGLTexture2D> mTexture;
    gl::Framebuffer* mPrevFramebuffer = nullptr;
    glm::uvec2 mPrevViewportSize = {1, 1};
    glm::mat4 mPrevProjectionMatrix = glm::mat4(1.0f);
    std::uint8_t mPrevStencilDepth = 0;
    APixelFormat mFormat;
};
