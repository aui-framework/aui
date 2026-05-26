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
#include <AUI/Render/SimpleTexturePacker.h>
#include "AUI/Render/ABorderStyle.h"
#include <AUI/Render/IRendererBackend.h>
#include "AUI/GL/RenderTarget/TextureRenderTarget.h"

class API_AUI_VIEWS OpenGLTexture2D : public ITexture, public gl::Framebuffer::IRenderTarget {
private:
    gl::Texture2D mTexture;
public:
    void setImage(AImageView image) override { mTexture.tex2D(image); }

    [[nodiscard]]
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
    friend class OpenGLPrerenderedString;
    friend class OpenGLMultiStringCanvas;
    friend class OpenGLRenderViewToTexture;
public:
    struct FontEntryData: aui::noncopyable {
        OpenGLRenderer* renderer;
        _<OpenGLTexture2D> texture;
        bool isTextureInvalid = true;
        Util::SimpleTexturePacker texturePacker;

        explicit FontEntryData(OpenGLRenderer* renderer): renderer(renderer), texture(_new<OpenGLTexture2D>()) {}
    };

    struct CharacterData {
        glm::vec4 uv;
    };

    struct FramebufferWithTextureRT {
        gl::Framebuffer framebuffer;
        _<gl::Framebuffer::IRenderTarget> rendertarget;
    };

    using OffscreenFramebufferPool = AVector<std::unique_ptr<FramebufferWithTextureRT>>;

    struct FramebufferBackToPool {
        OpenGLRenderer* renderer;
        void operator()(FramebufferWithTextureRT* framebuffer) const;
    };

    using FramebufferFromPool = _unique<FramebufferWithTextureRT, FramebufferBackToPool>;

    FramebufferFromPool getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize);

    // IRendererBackend implementation
    void solidRectangles(const ADisplayList::SolidRectangles& v, const glm::mat4& transform, Blending blending) override;
    void gradientRectangles(const ADisplayList::GradientRectangles& v, const glm::mat4& transform, Blending blending) override;
    void texturedRectangles(const ADisplayList::TexturedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void solidRoundedRectangles(const ADisplayList::SolidRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void gradientRoundedRectangles(const ADisplayList::GradientRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void texturedRoundedRectangles(const ADisplayList::TexturedRoundedRectangles& v, const glm::mat4& transform, Blending blending) override;
    void rectangleBorders(const ADisplayList::RectangleBorders& v, const glm::mat4& transform, Blending blending) override;
    void roundedRectangleBorders(const ADisplayList::RoundedRectangleBorders& v, const glm::mat4& transform, Blending blending) override;
    void boxShadow(const ADisplayList::BoxShadow& v, const glm::mat4& transform, Blending blending) override;
    void boxShadowInner(const ADisplayList::BoxShadowInner& v, const glm::mat4& transform, Blending blending) override;
    void glyphs(const ADisplayList::Glyphs& v, const glm::mat4& transform, Blending blending) override;
    _<IRenderer::IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;
    void lines(const ADisplayList::Lines& v, const glm::mat4& transform, Blending blending) override;
    void points(const ADisplayList::Points& v, const glm::mat4& transform, Blending blending) override;
    void lines(const ADisplayList::LineBatches& v, const glm::mat4& transform, Blending blending) override;
    void squareSector(const ADisplayList::SquareSector& v, const glm::mat4& transform, Blending blending) override;
    void backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) override;
    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) override;

    // Common
    _<ITexture> createTexture(glm::u32vec2 size) override;
    float getRenderScale() const noexcept override { return mRenderScale; }
    void setRenderScale(float renderScale) override { mRenderScale = renderScale; }
    void setAllowRenderToTexture(bool allow) override { mAllowRenderToTexture = allow; }
    bool allowRenderToTexture() const noexcept override { return mAllowRenderToTexture; }
    _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;
    void setWindow(ASurface* window) override { mWindow = window; }
    ASurface* getWindow() const noexcept override { return mWindow; }
    glm::mat4 getProjectionMatrix() const override;

    typedef void* (*GLLoadProc)(const char* name);
    static bool loadGL(GLLoadProc load_proc, bool es);
    static bool loadGL(GLLoadProc load_proc);

    static bool mIsES;
    static int mGLSLVersion;

    OpenGLRenderer();
    ~OpenGLRenderer() override = default;
    void identityUv();
    bool isVaoAvailable() const noexcept;
    void drawRectImpl(glm::vec2 position, glm::vec2 size);
    void setBlending(Blending blending);
    void beginPaint(glm::uvec2 windowSize);
    void endPaint();
    uint32_t getDefaultFb() const noexcept;
    void bindTemporaryVao() const noexcept { mBatchVao.bind(); }

    struct VertexBasic {
        glm::vec2 pos;
        glm::vec4 color;
    };
    struct VertexBasicUv {
        glm::vec2 pos;
        glm::vec2 uv;
        glm::vec4 color;
    };
    struct VertexSymbol {
        glm::vec2 pos;
        glm::vec2 uv;
        glm::vec4 color;
    };

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

protected:
    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);

private:
    ASurface* mWindow = nullptr;
    float mRenderScale = 1.0f;
    bool mAllowRenderToTexture = true;
    
    AOptional<gl::Program> mSolidShader;
    AOptional<gl::Program> mBoxShadowShader;
    AOptional<gl::Program> mBoxShadowInnerShader;
    AOptional<gl::Program> mRoundedSolidShader;
    AOptional<gl::Program> mRoundedSolidShaderBorder;
    AOptional<gl::Program> mGradientShader;
    AOptional<gl::Program> mRoundedGradientShader;
    AOptional<gl::Program> mTexturedShader;
    AOptional<gl::Program> mUnblendShader;
    AOptional<gl::Program> mSquareSectorShader;
    AOptional<gl::Program> mSymbolShader;
    AOptional<gl::Program> mSymbolShaderSubPixel;
    AOptional<gl::Program> mLineSolidDashedShader;

    gl::Vao mBatchVao;
    gl::Texture2D mGradientTexture;

    TransientBuffer mVertexBuffer;
    TransientBuffer mIndexBuffer;

    ADeque<FontEntryData> mFontEntryData;
    AVector<CharacterData> mCharData;
    OffscreenFramebufferPool mFramebuffersForMultiPassEffectsPool;

    struct {
        IRenderViewToTexture* mRenderToTextureTarget = nullptr;
    };
};
