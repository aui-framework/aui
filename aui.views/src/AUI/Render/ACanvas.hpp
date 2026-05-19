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

//
// Created by Nelonn on 5/20/2026.
//

#pragma once

#include <glm/glm.hpp>
#include <AUI/Common/AColor.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Render/ABorderStyle.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Font/AFontStyle.h>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Util/AAngleRadians.h>
#include <AUI/Util/AArrayView.h>
#include <AUI/Common/AString.h>
#include <AUI/ASS/Property/Backdrop.h>
#include <span>
#include "Blending.h"

/**
 * @brief Fully abstract drawing interface for AUI.
 * @details
 * ACanvas provides a set of high-level drawing primitives and state management.
 * It is designed to be recorded into a DisplayList or executed immediately by a Renderer.
 */
class ACanvas {
public:
    virtual ~ACanvas() = default;

    /**
     * @brief Saves the current state (transform, color, stencil depth) onto the stack.
     */
    virtual void save() = 0;

    /**
     * @brief Restores the state from the stack.
     */
    virtual void restore() = 0;

    /**
     * @brief Pushes a new rendering layer.
     * @details
     * Subsequent drawing calls will be rendered to a temporary buffer and then blended back to the parent layer.
     * This is useful for opacity and complex effects.
     */
    virtual void pushLayer() = 0;

    /**
     * @brief Pops the current rendering layer and blends it back.
     */
    virtual void popLayer() = 0;

    /**
     * @brief Sets the transform matrix which is applicable for any figure. Unlike <code>setTransformForced</code>, the new
     * matrix is multiplied by the previous matrix.
     * @param transform transform matrix
     */
    virtual void setTransform(const glm::mat4& transform) = 0;

    /**
     * @brief Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
     * by the previous color.
     * @param color color
     */
    virtual void setColor(const AColor& color) = 0;

    /**
     * @brief Sets blending mode.
     * @param blending new blending mode
     * @details
     * **Blending Modes and Effects**
     *
     * The rendering engine supports multiple blending modes for advanced visual effects:
     *
     * <!-- aui:steal_documentation Blending -->
     */
    virtual void setBlending(Blending blending) = 0;

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
     * @param offset shadow offset. Unlike outer shadow (ctx.render.boxShadow), the offset is passed to the shader instead
     *               of a simple rectangle position offset.
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
     * @param position string baseline
     * @param string string to render
     * @param fs font style (optional)
     * @details
     * This function is dramatically inefficient since it does symbol lookup for every character is the
     * <code>string</code> and does GPU buffer allocations. If you want to render the same string for several
     * times (frames), consider using the IRenderer::prerenderString function or high level views (such as
     * ALabel) instead.
     */
    virtual void string(glm::vec2 position,
                        const AString& string,
                        const AFontStyle& fs = {}) = 0;

    /**
     * @brief Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     * @param style style
     * @param width line width
     */
    virtual void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) = 0;

    /**
     * @brief Draws points list.
     * @param brush brush
     * @param points points
     * @param size point size
     */
    virtual void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) = 0;

    /**
     * @brief Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     * @param style style
     * @param width line width
     */
    virtual void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) = 0;

    /**
     * @brief Draws sector in rectangle shape. The sector is drawn clockwise from begin to end angles.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param begin begin angle of the sector
     * @param end end angle of the sector
     * @details
     * The method can be used as mask to ctx.render.roundedRect, creating arc shape.
     */
    virtual void squareSector(const ABrush& brush,
                              const glm::vec2& position,
                              const glm::vec2& size,
                              AAngleRadians begin,
                              AAngleRadians end) = 0;

    /**
     * @brief witches drawing to the stencil buffer instead of color buffer.
     * @details
     * Stencil pixel is increased by each affected pixel.
     * Should be called before the <code>pushMaskAfter</code> function.
     */
    virtual void pushMaskBefore() = 0;

    /**
     * @brief Switches drawing to the color buffer back from the stencil. Increases stencil depth.
     * @details
     * Stencil buffer should not be changed after calling this function.
     * Should be called after the <code>pushMaskBefore</code> function.
     */
    virtual void pushMaskAfter() = 0;

    /**
    * @brief Switches drawing to the stencil buffer instead of color buffer.
    * @details
    * Stencil pixel is decreased by each affected pixel.
    * Should be called before the <code>popMaskAfter</code> function.
    */
    virtual void popMaskBefore() = 0;

    /**
     * @brief Switches drawing to the color buffer back from the stencil. Decreases stencil depth.
     * @details
     * Stencil buffer should not be changed after calling this function.
     * Should be called after the <code>popMaskBefore</code> function.
     */
    virtual void popMaskAfter() = 0;

    /**
     * @brief Draws rectangular backdrop effects.
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param backdrops array of backdrop effects. Impl might apply optimizations on using several effects at once.
     * @details
     * Implementation might draw stub (i.e., gray rectangle) instead of drawing complex backdrop effects.
     */
    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Any> backdrops) = 0;
};
