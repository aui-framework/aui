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
#include "AUI/Render/ABorderStyle.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/GL/RenderTarget/TextureRenderTarget.h"

class API_AUI_VIEWS OpenGLRenderer final: public IRenderer {
    friend class OpenGLPrerenderedString;
    friend class OpenGLMultiStringCanvas;
public:
    struct FontEntryData: aui::noncopyable {
        Util::SimpleTexturePacker texturePacker;
        gl::Texture2D texture;
        bool isTextureInvalid = true;

        FontEntryData() {
            texture.bind();
            texture.setupNearest();
        }
    };

    using GLLoadProc = void* (*) (const char* name);

    /**
     * @brief Manually specify is ES or not
     */
    static bool loadGL(GLLoadProc load_proc, bool es);

    /**
     * @brief Automatically detects ES or not
     */
    static bool loadGL(GLLoadProc load_proc);


private:
    AOptional<gl::Program> mSolidShader;
    AOptional<gl::Program> mGradientShader;
    AOptional<gl::Program> mRoundedSolidShader;
    AOptional<gl::Program> mRoundedSolidShaderBorder;
    AOptional<gl::Program> mRoundedGradientShader;
    AOptional<gl::Program> mBoxShadowShader;
    AOptional<gl::Program> mBoxShadowInnerShader;
    AOptional<gl::Program> mTexturedShader;
    AOptional<gl::Program> mUnblendShader;
    AOptional<gl::Program> mSymbolShader;
    AOptional<gl::Program> mSymbolShaderSubPixel;
    AOptional<gl::Program> mSquareSectorShader;
    AOptional<gl::Program> mLineSolidDashedShader;
    gl::Vao mRectangleVao;
    gl::Vao mBorderVao;
    gl::Texture2D mGradientTexture;


    struct CharacterData {
        glm::vec4 uv;
    };

    ADeque<CharacterData> mCharData;
    ADeque<FontEntryData> mFontEntryData;
    IRenderViewToTexture* mRenderToTextureTarget = nullptr;

    struct FramebufferWithTextureRT {
        gl::Framebuffer framebuffer;
        _<gl::TextureRenderTarget<gl::InternalFormat::RGBA8, gl::Type::UNSIGNED_BYTE, gl::Format::RGBA>> rendertarget;
    };

    struct FramebufferBackToPool {
        OpenGLRenderer* renderer;
        void operator()(FramebufferWithTextureRT* framebuffer) const;
    };

    using FramebufferFromPool = std::unique_ptr<FramebufferWithTextureRT, FramebufferBackToPool>;
    using OffscreenFramebufferPool = AVector<FramebufferFromPool>;

    /**
     * @brief use getFramebufferForMultiPassEffect
     */
    OffscreenFramebufferPool mFramebuffersForMultiPassEffectsPool;


    static std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 position, glm::vec2 size);

    void uploadToShaderCommon();

    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);

    /**
     * @return true, if the caller should compute distances
     */
    bool setupLineShader(const ABrush& brush, const ABorderStyle& style, float widthPx);


    /**
     * @brief get a framebuffer for rendering multi pass effects (i.e., blur)
     * @param minRequiredSize minimum required size of the framebuffer
     * @return framebuffer, or null if unsupported
     * @details
     * Returns a shared color-only framebuffer that can be used for rendering multi pass effects. The size of
     * framebuffer is guaranteed to be no lower than minRequiredSize. Generally, the buffer would be larger than
     * requested.
     *
     * Buffer may contain dirty data.
     *
     * The function acts like pool aggregator.
     *
     * The returned framebuffer object is wrapped by smart pointer. Caller takes unique ownership of the framebuffer.
     * When smart pointer is destroyed, the framebuffer object is returned back to the pool. Thus, while caller owns
     * framebuffer object, the function would never return the same object until caller releases ownership.
     */
    FramebufferFromPool getFramebufferForMultiPassEffect(glm::uvec2 minRequiredSize);

    void backdrops(glm::ivec2 fbSize, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops) override;

protected:
    _unique<ITexture> createNewTexture() override;

public:
    OpenGLRenderer();
    ~OpenGLRenderer() override = default;
    void identityUv();
    bool isVaoAvailable() const noexcept;

    void rectangle(const ABrush& brush,
                   glm::vec2 position,
                   glm::vec2 size) override;

    void roundedRectangle(const ABrush& brush,
                          glm::vec2 position,
                          glm::vec2 size,
                          float radius) override;

    void rectangleBorder(const ABrush& brush,
                         glm::vec2 position,
                         glm::vec2 size,
                         float lineWidth) override;

    void roundedRectangleBorder(const ABrush& brush,
                                glm::vec2 position,
                                glm::vec2 size,
                                float radius,
                                int borderWidth) override;

    void boxShadow(glm::vec2 position,
                   glm::vec2 size,
                   float blurRadius,
                   const AColor& color) override;
        
    void boxShadowInner(glm::vec2 position,
                        glm::vec2 size,
                        float blurRadius,
                        float spreadRadius,
                        float borderRadius,
                        const AColor& color,
                        glm::vec2 offset) override;

    void string(glm::vec2 position,
                const AString& string,
                const AFontStyle& fs) override;

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void drawRectImpl(glm::vec2 position, glm::vec2 size);

    void setBlending(Blending blending) override;

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

    glm::mat4 getProjectionMatrix() const override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;

    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;

    void squareSector(const ABrush& brush,
                      const glm::vec2& position,
                      const glm::vec2& size,
                      AAngleRadians begin,
                      AAngleRadians end) override;

    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;

    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;

    void popMaskAfter() override;

    void beginPaint(glm::uvec2 windowSize);
    void endPaint();
    
    uint32_t getDefaultFb() const noexcept;
    void bindTemporaryVao() const noexcept;
};


