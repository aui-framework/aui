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


#include <AUI/Render/IRenderer.h>
#include <AUI/Platform/ASurface.h>
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
        AUI_ASSERTX(mContext != nullptr, "context is null");
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

    _<IPrerenderedString> prerenderString(glm::vec2 position,
                                          const AString& text,
                                          const AFontStyle& fs) override;

    void setBlending(Blending blending) override;

    void setWindow(ASurface* window) override;

    glm::mat4 getProjectionMatrix() const override;

    void pushMaskBefore() override;
    void pushMaskAfter() override;
    void popMaskBefore() override;
    void popMaskAfter() override;

    _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept override;

    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) override;
    void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) override;

    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) override;

    void squareSector(const ABrush& brush,
                      const glm::vec2& position,
                      const glm::vec2& size,
                      AAngleRadians begin,
                      AAngleRadians end) override;
protected:
    _unique<ITexture> createNewTexture() override;

    void drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style, AMetric width);

};


