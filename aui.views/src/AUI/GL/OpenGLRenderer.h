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
            gl::Texture2D::setupNearest();
        }
    };

private:
    gl::Program mSolidShader;
    gl::Program mGradientShader;
    gl::Program mRoundedSolidShader;
    gl::Program mRoundedSolidShaderBorder;
    gl::Program mRoundedGradientShader;
    gl::Program mBoxShadowShader;
    gl::Program mTexturedShader;
    gl::Program mSymbolShader;
    gl::Program mSymbolShaderSubPixel;
    gl::Program mSquareSectorShader;
    gl::Vao mTempVao;
    gl::Texture2D mGradientTexture;

    struct NotTried{}; struct Failed{}; std::variant<NotTried, Failed, gl::Framebuffer> mFramebuffer;


    struct CharacterData {
        glm::vec4 uv;
    };

    ADeque<CharacterData> mCharData;
    ADeque<FontEntryData> mFontEntryData;


    std::array<glm::vec2, 4> getVerticesForRect(glm::vec2 position,
                                          glm::vec2 size);

    void uploadToShaderCommon();
    void identityUv();

    void endDraw(const ABrush& brush);
    void tryEnableFramebuffer(glm::uvec2 windowSize);
    FontEntryData* getFontEntryData(const AFontStyle& fontStyle);
protected:
    ITexture* createNewTexture() override;

public:
    OpenGLRenderer();
    ~OpenGLRenderer() override = default;
    bool isVaoAvailable();

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

    void drawString(glm::vec2 position,
                    const AString& string,
                    const AFontStyle& fs) override;

    _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) override;

    void drawRectImpl(glm::vec2 position, glm::vec2 size);

    void setBlending(Blending blending) override;

    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

    glm::mat4 getProjectionMatrix() const override;

    void drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2) override;

    void drawLines(const ABrush& brush, AArrayView<glm::vec2> points) override;

    void drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points) override;

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
};


