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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Common/ASide.h>
#include <AUI/Common/AColor.h>
#include <AUI/Render/APaint.hpp>
#include <AUI/Util/AArrayView.h>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/Util/AMetric.h>
#include "ATextLayoutHelper.h"
#include <AUI/Font/FontRendering.hpp>
#include <AUI/Render/IRendererText.hpp>
#include <AUI/Render/IRenderViewToTexture.h>
#include <AUI/ASS/Property/Backdrop.h>

class AColor;
class ASurface;
class AFontStyle;
class ACanvas;

/**
 * @brief Base class for renderer.
 * @ingroup views
 */
class IRenderer: public aui::noncopyable {
public:
    using IPrerenderedString = aui::IPrerenderedString;
    using IMultiStringCanvas = aui::IMultiStringCanvas;

public:
    IRenderer() = default;
    virtual ~IRenderer() = default;

    /**
     * @brief Creates new canvas for batching multiple <code>prerender</code> string calls.
     * @return a new instance of <code>IMultiStringCanvas</code>
     */
    virtual _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    /**
     * @brief Draws simple rectangle.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     */
    virtual void rectangle(const ABrush& brush,
                           glm::vec2 position,
                           glm::vec2 size) = 0;


    /**
     * @brief Draws rounded rect (with antialiasing, if msaa enabled).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    virtual void roundedRectangle(const ABrush& brush,
                                  glm::vec2 position,
                                  glm::vec2 size,
                                  float radius) = 0;

    /**
     * @brief Draws rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param lineWidth border line width (px)
     */
    virtual void rectangleBorder(const ABrush& brush,
                                 glm::vec2 position,
                                 glm::vec2 size,
                                 float lineWidth = 1.f) = 0;
    /**
     * @brief Draws rounded rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     * @param borderWidth border line width (px)
     */
    virtual void roundedRectangleBorder(const ABrush& brush,
                                        glm::vec2 position,
                                        glm::vec2 size,
                                        float radius,
                                        int borderWidth) = 0;


    /**
     * @brief Draws a rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param color shadow color
     */
    virtual void boxShadow(glm::vec2 position,
                           glm::vec2 size,
                           float blurRadius,
                           const AColor& color) = 0;

    /**
     * @brief Draws inner (inset) rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param spreadRadius spread (offset) radius
     * @param borderRadius border radius of the rectangle.
     * @param color shadow color
     * @param offset shadow offset.
     */
    virtual void boxShadowInner(glm::vec2 position,
                                glm::vec2 size,
                                float blurRadius,
                                float spreadRadius,
                                float borderRadius,
                                const AColor& color,
                                glm::vec2 offset) = 0;

    /**
     * @brief Draws string.
     */
    virtual void string(glm::vec2 position,
                        const AString& string,
                        const AFontStyle& fs) = 0;

    /**
     * @brief Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code>.
     */
    virtual _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;

    virtual void line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) = 0;

    virtual void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) = 0;

    virtual void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style = ABorderStyle::Solid{}) = 0;

    virtual void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) = 0;

    virtual void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) = 0;

    virtual void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style = ABorderStyle::Solid{}) = 0;

    virtual void squareSector(const ABrush& brush,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              AAngleRadians begin,
                              AAngleRadians end) = 0;
 
    virtual void setColorForced(const AColor& color) = 0;

    virtual void setColor(const AColor& color) = 0;

    virtual const AColor& getColor() const = 0;

    virtual void setTransform(const glm::mat4& transform) = 0;

    virtual void setTransformForced(const glm::mat4& transform) = 0;

    virtual void pushMaskBefore() = 0;

    virtual void pushMaskAfter() = 0;

    virtual void popMaskBefore() = 0;

    virtual void popMaskAfter() = 0;

    virtual void setBlending(Blending blending) = 0;

    [[nodiscard]]
    virtual _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept = 0;

    virtual void setWindow(ASurface* window) = 0;

    [[nodiscard]]
    virtual ASurface* getWindow() const noexcept = 0;

    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual glm::mat4 getTransform() = 0;

    [[nodiscard]]
    virtual std::uint8_t getStencilDepth() const noexcept = 0;

    virtual void setStencilDepth(uint8_t stencilDepth) = 0;


    virtual void translate(const glm::vec2& offset) = 0;

    virtual void rotate(const glm::vec3& axis, AAngleRadians angle) = 0;

    virtual void rotate(AAngleRadians angle) = 0;

    virtual void setAllowRenderToTexture(bool allowRenderToTexture) = 0;

    [[nodiscard]]
    virtual bool allowRenderToTexture() const noexcept = 0;

    virtual void setRenderScale(float render_scale) = 0;

    virtual float getRenderScale() const noexcept = 0;

    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Any> backdrops) = 0;

    virtual void stub(glm::vec2 position, glm::vec2 size) = 0;

    virtual ACanvas& canvas() = 0;
};
