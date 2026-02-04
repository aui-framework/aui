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


#include "AUI/GL/Program.h"
#include "AUI/GL/Framebuffer.h"
#include "AUI/GL/Vao.h"
#include "AUI/Render/ABorderStyle.h"
#include "AUI/GL/RenderTarget/TextureRenderTarget.h"
#include "IBatchingRenderer.h"

class OpenGLRenderer final: public IBatchingRenderer {
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

    OpenGLRenderer();
    ~OpenGLRenderer() override = default;

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;
    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void beginPaint(glm::uvec2 windowSize);
    void endPaint();

    void identityUv();

protected:
    _unique<ITexture> createNewTexture() override;
    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;

    void handleCmds(std::vector<Cmd> cmds) override;

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

    void drawRectImpl(glm::vec2 position, glm::vec2 size);


private:
    // Helper methods that perform the actual rendering for each command.
    void renderRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size);
    void renderRoundedRectangle(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius);
    void renderRectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float lineWidth);
    void renderRoundedRectangleBorder(const ABrush& brush, glm::vec2 position, glm::vec2 size, float radius, int borderWidth);
    void renderBoxShadow(glm::vec2 position, glm::vec2 size, float blurRadius, const AColor& color);
    void renderBoxShadowInner(glm::vec2 position, glm::vec2 size, float blurRadius, float spreadRadius, float borderRadius, const AColor& color, glm::vec2 offset);
    void renderString(glm::vec2 position, const AString& string, const AFontStyle& fs);
    void renderLines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width);
    void renderLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width);
    void renderPoints(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size);
    void renderLinesPairs(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width);
    void renderSquareSector(const ABrush& brush, const glm::vec2& position, const glm::vec2& size, AAngleRadians begin, AAngleRadians end);
    void renderPushMaskBefore();
    void renderPopMaskBefore();
    void renderPushMaskAfter();
    void renderPopMaskAfter();
    void renderSetBlending(Blending blending);
    void renderBackdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops);

    void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops) override;

    glm::mat4 getProjectionMatrix() const;

    void bindTemporaryVao() const noexcept;
    bool isVaoAvailable() const noexcept;
};


