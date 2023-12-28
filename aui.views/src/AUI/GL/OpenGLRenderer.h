// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include <AUI/GL/Program.h>
#include <AUI/GL/Framebuffer.h>
#include <AUI/GL/Vao.h>
#include "AUI/Render/ABorderStyle.h"
#include "AUI/Render/IRenderer.h"

class OpenGLRenderer: public IRenderer {
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

private:
    AOptional<gl::Program> mSolidShader;
    AOptional<gl::Program> mGradientShader;
    AOptional<gl::Program> mRoundedSolidShader;
    AOptional<gl::Program> mRoundedSolidShaderBorder;
    AOptional<gl::Program> mRoundedGradientShader;
    AOptional<gl::Program> mBoxShadowShader;
    AOptional<gl::Program> mBoxShadowInnerShader;
    AOptional<gl::Program> mTexturedShader;
    AOptional<gl::Program> mSymbolShader;
    AOptional<gl::Program> mSymbolShaderSubPixel;
    AOptional<gl::Program> mSquareSectorShader;
    AOptional<gl::Program> mLineSolidDashedShader;
    gl::Vao mRectangleVao;
    gl::Vao mBorderVao;
    gl::Texture2D mGradientTexture;

    glm::uvec2 mViewportSize;

    struct NotTried{}; struct Failed{}; std::variant<NotTried, Failed, gl::Framebuffer> mFramebuffer;


    struct CharacterData {
        glm::vec4 uv;
    };

    ADeque<CharacterData> mCharData;
    ADeque<FontEntryData> mFontEntryData;


    std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 position,
                                          glm::vec2 size);

    void uploadToShaderCommon();

    void tryEnableFramebuffer(glm::uvec2 windowSize);
    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);

    /**
     * @return true, if the caller should compute distances
     */
    bool setupLineShader(const ABrush& brush, const ABorderStyle& style, float widthPx);

protected:
    ITexture* createNewTexture() override;

public:
    OpenGLRenderer();
    ~OpenGLRenderer() override = default;
    void identityUv();
    bool isVaoAvailable() const noexcept;

    void drawRect(const ABrush& brush,
                  glm::vec2 position,
                  glm::vec2 size) override;

    void drawRoundedRect(const ABrush& brush,
                         glm::vec2 position,
                         glm::vec2 size,
                         float radius) override;

    void drawRectBorder(const ABrush& brush,
                       glm::vec2 position,
                       glm::vec2 size,
                       float lineWidth) override;

    void drawRoundedRectBorder(const ABrush& brush,
                               glm::vec2 position,
                               glm::vec2 size,
                               float radius,
                               int borderWidth) override;

    void drawBoxShadow(glm::vec2 position,
                       glm::vec2 size,
                       float blurRadius,
                       const AColor& color) override;
        
    void drawBoxShadowInner(glm::vec2 position,
                            glm::vec2 size,
                            float blurRadius,
                            float spreadRadius,
                            float borderRadius,
                            const AColor& color,
                            glm::vec2 offset) override;   

    void drawString(glm::vec2 position,
                    const AString& string,
                    const AFontStyle& fs) override;

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void drawRectImpl(glm::vec2 position, glm::vec2 size);

    void setBlending(Blending blending) override;

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

    glm::mat4 getProjectionMatrix() const override;

    void drawLines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;

    void drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void drawSquareSector(const ABrush& brush,
                          const glm::vec2& position,
                          const glm::vec2& size,
                          AAngleRadians begin,
                          AAngleRadians end) override;

    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;

    void beginPaint(glm::uvec2 windowSize);
    void endPaint();
    
    uint32_t getDefaultFb() const noexcept;
    void bindViewport() const noexcept;
    void bindTemporaryVao() const noexcept;

    [[nodiscard]]
    uint32_t getSupersamplingRatio() const noexcept;

    [[nodiscard]]
    glm::uvec2 viewportSize() const noexcept {
        return mViewportSize;
    }
};


