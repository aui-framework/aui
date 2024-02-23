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


#include <AUI/Render/IRenderer.h>
#include <AUI/Platform/ABaseWindow.h>
#include <AUI/Platform/SoftwareRenderingContext.h>

class API_AUI_VIEWS SoftwareRenderer: public IRenderer {
private:
    SoftwareRenderingContext* mContext;
    bool mDrawingToStencil = false;
    enum {
        INCREASE = 1,
        DECREASE = -1
    } mDrawingStencilDirection;
    Blending mBlending = Blending::NORMAL;

public:
    /**
     * Draws a pixel onto the software framebuffer following the stencil and blending rules.
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>position</code> is inside the framebuffer.</dd>
     * </dl>
     * @param position position. An assertion is triggered if position is not inside the framebuffer.
     * @param color color.
     * @param blending blending. Optional. Cheaper. When set, the one set by the <code>setBlending</code> function is
     *        ignored.
     */
    inline void putPixel(glm::ivec2 position, AColor color, AOptional<Blending> blending = std::nullopt) noexcept {
        assert(("context is null" && mContext != nullptr));
        color = glm::clamp(color, glm::vec4(0), glm::vec4(1));
        auto actualBlending = blending ? *blending : mBlending;
        glm::uvec2 uposition(position);
        if (!glm::all(glm::lessThan(uposition, mContext->bitmapSize()))) return;

        if (mDrawingToStencil) {
            if (color.a > 0.5f) {
                mContext->stencil(position) += mDrawingStencilDirection;
            }
        } else {
            auto bufferStencilValue = mContext->stencil(position);
            if (bufferStencilValue == mStencilDepth)
            {
                switch (actualBlending) {
                    case Blending::NORMAL:
                        if (color.a >= 0.9999f) {
                            mContext->putPixel(uposition, glm::u8vec4(glm::vec4(color) * 255.f));
                        } else {
                            // blending
                            auto u8srcColor = mContext->getPixel(uposition);
                            if (u8srcColor.a == 0) {
                                // put the color "as is"
                                mContext->putPixel(uposition, glm::u8vec4(color * 255.f));
                            } else {
                                auto srcColor = glm::vec3(u8srcColor.r, u8srcColor.g, u8srcColor.b);
                                if (u8srcColor.a == 255) {
                                    mContext->putPixel(uposition,
                                                       glm::u8vec4(glm::mix(srcColor, glm::vec3(color) * 255.f, color.a), 255));
                                } else {
                                    // blend with the src color; calculate final alpha
                                    auto dstColor = glm::vec3(color) * 255.f;
                                    auto srcAlpha = float(u8srcColor.a) / 255.f;
                                    float finalAlpha = srcAlpha + (1.f - srcAlpha) * color.a;
                                    mContext->putPixel(uposition,
                                                       glm::u8vec4(glm::u8vec3(srcColor * srcAlpha + dstColor * color.a), uint8_t(finalAlpha * 255.f)));
                                }
                            }
                        }
                        break;

                    case Blending::ADDITIVE: {
                        auto src = glm::uvec4(glm::vec4(color) * 255.f);
                        src.a = (src.x + src.y + src.z) / 3.f;
                        auto dst = glm::uvec4(mContext->getPixel(uposition));
                        mContext->putPixel(uposition, glm::u8vec4((glm::min)(src + dst, glm::uvec4(255))));
                        break;
                    }
                    case Blending::INVERSE_DST: {
                        auto src = glm::vec3(color);
                        auto dst = glm::vec3(mContext->getPixel(uposition)) / 255.f;
                        mContext->putPixel(uposition, (glm::min)(glm::uvec3((src * (1.f - dst)) * 255.f), glm::uvec3(255)));
                        break;
                    }
                    case Blending::INVERSE_SRC:
                        auto src = glm::vec3(color);
                        auto dstA = glm::vec4(mContext->getPixel(uposition)) / 255.f;
                        auto dst = glm::vec3(dstA);
                        mContext->putPixel(uposition, glm::u8vec4((glm::min)(glm::uvec3(((1.f - src) * dst) * 255.f), glm::uvec3(255)), glm::clamp(color.x + color.y + color.z, dstA.a, 1.f) * 255));
                        break;
                }
            }
        }
    }
    _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) override;

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

    _<IPrerenderedString> prerenderString(glm::vec2 position,
                                          const AString& text,
                                          const AFontStyle& fs) override;

    void setBlending(Blending blending) override;

    void setWindow(ABaseWindow* window) override;

    glm::mat4 getProjectionMatrix() const override;

    void pushMaskBefore() override;

    void pushMaskAfter() override;

    void popMaskBefore() override;

    void popMaskAfter() override;


    void drawLines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;
    void drawPoints(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;

    void drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void drawSquareSector(const ABrush& brush,
                          const glm::vec2& position,
                          const glm::vec2& size,
                          AAngleRadians begin,
                          AAngleRadians end) override;
protected:
    ITexture* createNewTexture() override;

    void drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style, AMetric width);

};


