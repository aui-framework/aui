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

    bool setupLineShader(const glm::mat4& transform, const ABorderStyle& style, float widthPx);
    std::uint8_t mStencilDepth = 0;

    ASurface* mWindow = nullptr;
    bool mAllowRenderToTexture = true;

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

    TransientBuffer mVertexBuffer;
    TransientBuffer mIndexBuffer;

    _<aui::AFontCache> mFontCache;
    OffscreenFramebufferPool mFramebuffersForMultiPassEffectsPool;

    glm::uvec2 mViewportSize = { 1, 1 };
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);

    IRenderViewToTexture* mRenderToTextureTarget = nullptr;

public:
    typedef void* (*GLLoadProc)(const char* name);
    static bool loadGL(GLLoadProc load_proc, bool es);
    static bool loadGL(GLLoadProc load_proc);

    static bool mIsES;
    static int mGLSLVersion;

    OpenGLRenderer();
    ~OpenGLRenderer() override = default;

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
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, const APaint& paint) override;
    void points(const ADisplayList::Points& v, const glm::mat4& transform, const APaint& paint) override;
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, const APaint& paint) override;
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, const APaint& paint) override;
    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;

    _<ITexture> createTexture(glm::u32vec2 size, APixelFormat format = APixelFormat::RGBA_BYTE, TextureFilter filter = TextureFilter::LINEAR) override;
    void setAllowRenderToTexture(bool allow) override { mAllowRenderToTexture = allow; }
    bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }
    void setWindow(ASurface* window) override;
    ASurface* getWindow() const noexcept override { return mWindow; }
    glm::mat4 getProjectionMatrix() const override;

    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;

    const _<aui::AFontCache>& getFontCache() override { return mFontCache; }

    bool isVaoAvailable() const noexcept;
    void setBlending(const APaint& paint);
    void beginPaint(glm::uvec2 windowSize);
    void endPaint();
    uint32_t getDefaultFb() const noexcept;
    void bindTemporaryVao() const noexcept { mBatchVao.bind(); }

    FramebufferFromPool getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize);
};

class OpenGLRenderViewToTexture: public IRenderViewToTexture {
public:
    OpenGLRenderViewToTexture(OpenGLRenderer& renderer): mRenderer(renderer) {}

    bool begin(IRenderer& renderer, glm::ivec2 surfaceSize, InvalidArea& invalidArea) override {
        if (!mTexture || mTexture->getSize() != glm::u32vec2(surfaceSize)) {
            mTexture = _cast<OpenGLTexture2D>(mRenderer.createTexture(surfaceSize));
            mFramebuffer = gl::Framebuffer();
            mFramebuffer.resize(surfaceSize);
            mFramebuffer.attach(mTexture, GL_COLOR_ATTACHMENT0);
            auto stencil = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::DEPTH24_STENCIL8, gl::Multisampling::DISABLED>>();
            mFramebuffer.attach(stencil, GL_DEPTH_STENCIL_ATTACHMENT);
            invalidArea = InvalidArea::Full{};
        }

        mPrevFramebuffer = gl::Framebuffer::current();
        mFramebuffer.bind();
        glViewport(0, 0, surfaceSize.x, surfaceSize.y);
        mRenderer.mProjectionMatrix = glm::ortho(0.f, (float)surfaceSize.x, (float)surfaceSize.y, 0.f, -1.f, 1.f);

        if (invalidArea.full()) {
            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        return true;
    }

    void end(IRenderer& renderer) override {
        if (mPrevFramebuffer) {
            mPrevFramebuffer->bind();
        } else {
            gl::Framebuffer::unbind();
        }
        mPrevFramebuffer = nullptr;
        if (mRenderer.mWindow) {
            mRenderer.beginPaint(mRenderer.mWindow->getSize());
        }
    }

    void draw(ACanvas& canvas) override {
        canvas.rectangle(APaint{ATexturedBrush{mTexture}}, {0, 0}, mTexture->getSize());
    }

private:
    OpenGLRenderer& mRenderer;
    gl::Framebuffer mFramebuffer;
    _<OpenGLTexture2D> mTexture;
    gl::Framebuffer* mPrevFramebuffer = nullptr;
};
