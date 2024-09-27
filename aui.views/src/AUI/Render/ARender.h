/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Util/AArrayView.h>
#include "AUI/Render/ABorderStyle.h"
#include "AUI/Util/AMetric.h"
#include "IRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AUI/Util/AAngleRadians.h>

/**
 * @brief Rendering facade for IRenderer.
 * @ingroup views
 */
class API_AUI_VIEWS ARender
{
private:
    static _<IRenderer> ourRenderer;

public:
    using PrerenderedString = _<IRenderer::IPrerenderedString>;
    using Texture = _<ITexture>;

    static void setRenderer(_<IRenderer> renderer);

    static const _<IRenderer>& getRenderer() {
        return ourRenderer;
    }

    /**
     * @brief Creates new canvas for batching multiple <code>prerender</code> string calls.
     * @return a new instance of <code>IMultiStringCanvas</code>
     */
    static _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& fontStyle) {
        return ourRenderer->newMultiStringCanvas(fontStyle);
    }

    /**
     * @brief Creates new texture (image representation optimized for GPU rendering).
     */
    static Texture getNewTexture() {
        return ourRenderer->getNewTexture();
    }

    /**
     * @brief Draws simple rectangle.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     */
    static void rect(const ABrush& brush,
                     const glm::vec2& position,
                     const glm::vec2& size) {
        ourRenderer->rectangle(brush, position, size);
    }


    /**
     * @brief Draws rounded rect (with antialiasing, if msaa enabled).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    static void roundedRectangle(const ABrush& brush,
                            const glm::vec2& position,
                            const glm::vec2& size,
                            float radius) {
        ourRenderer->roundedRectangle(brush, position, size, radius);
    }

    /**
     * @brief Draws rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param lineWidth border line width (px)
     */
    static void rectangleBorder(const ABrush& brush,
                           const glm::vec2& position,
                           const glm::vec2& size,
                           float lineWidth = 1.f) {
        ourRenderer->rectangleBorder(brush, position, size, lineWidth);
    }

    /**
     * @brief Draws rounded rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     * @param borderWidth border line width (px)
     */
    static void roundedRectBorder(const ABrush& brush,
                                  const glm::vec2& position,
                                  const glm::vec2& size,
                                  float radius,
                                  int borderWidth) {
        ourRenderer->roundedRectangleBorder(brush, position, size, radius, borderWidth);
    }

    /**
     * @brief Draws a line between <code>p1</code> and <code>p2</code>.
     * @param brush brush
     * @param p1 first point
     * @param p2 second point
     * @param style style
     * @param width line width
     * @details
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>if you want to drawElements multiple lines, consider using <code>ctx.render.lines</code> function instead.</dd>
     * </dl>
     */
    static void line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) {
        glm::vec2 points[] = { p1, p2 };
        ourRenderer->lines(brush, points, style, width);
    }

    /**
     * @brief Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     * @param style style
     * @param width line width
     */
    static void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) {
        ourRenderer->lines(brush, points, style, width);
    }

    /**
     * @brief Draws points list.
     * @param brush brush
     * @param points points
     * @param size point size
     */
    static void points(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size = 1_dp) {
        ourRenderer->points(brush, points, size);
    }

    /**
     * @brief Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     * @param style style
     * @param width line width
     */
    static void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) {
        ourRenderer->lines(brush, points, style, width);
    }

    /**
     * @brief Draws a rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param color shadow color
     */
    static void boxShadow(const glm::vec2& position,
                          const glm::vec2& size,
                          float blurRadius,
                          const AColor& color) {
        ourRenderer->boxShadow(position, size, blurRadius, color);
    }

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
    static void boxShadowInner(glm::vec2 position,
                               glm::vec2 size,
                               float blurRadius,
                               float spreadRadius,
                               float borderRadius,
                               const AColor& color,
                               glm::vec2 offset) {
        ourRenderer->boxShadowInner(position, size, blurRadius, spreadRadius, borderRadius, color, offset);
    }


    /**
     * @brief Draws string.
     * @param position string's top left point
     * @param string string to render
     * @param fs font style (optional)
     * @details
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>
     *         This function is dramatically inefficient since it does symbol lookup for every character is the
     *         <code>string</code> and does GPU buffer allocations. If you want to render the same string for several
     *         times (frames), consider using the IRenderer::prerenderString function or high level views (such as
     *         ALabel) instead.
     *     </dd>
     * </dl>
     */
    static void string(const glm::vec2& position,
                       const AString& string,
                       const AFontStyle& fs = {}) {
        ourRenderer->string(position, string, fs);
    }

    /**
     * @brief Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param position string's top left point
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    static _<IRenderer::IPrerenderedString> prerenderString(const glm::vec2& position, const AString& text, AFontStyle& fs) {
        AUI_ASSERTX(!text.empty(), "empty string could not be prerendered");
        return ourRenderer->prerenderString(position, text, fs);
    }

    /**
     * @brief Draws sector in rectangle shape. The sector is drawn clockwise from begin to end angles.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @details
     * The method can be used as mask to ctx.render.roundedRect, creating arc shape.
     */
    static void squareSector(const ABrush& brush,
                             const glm::vec2& position,
                             const glm::vec2& size,
                             AAngleRadians begin,
                             AAngleRadians end) {
        ourRenderer->squareSector(brush, position, size, begin, end);
    }

    /**
     * @brief Sets the color which is multiplied with any brush.
     * @param color color
     */
    static void setColorForced(const AColor& color)
    {
        ourRenderer->setColorForced(color);
    }

    /**
     * @bruef Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
     * by the previous color.
     * @param color color
     */
    static void setColor(const AColor& color)
    {
        ourRenderer->setColor(color);
    }

    static const AColor& getColor()
    {
        return ourRenderer->getColor();
    }

    /**
     * @brief Sets the transform matrix which is applicable for any figure. Unlike <code>setTransformForced</code>, the new
     * matrix is multiplied by the previous matrix.
     * @param transform transform matrix
     */
    static void setTransform(const glm::mat4& transform)
    {
        ourRenderer->setTransform(transform);
    }

    /**
     * @brief Sets the transform matrix which is applicable for any figure.
     * @param transform transform matrix
     */
    static void setTransformForced(const glm::mat4& transform)
    {
        ourRenderer->setTransformForced(transform);
    }

    /**
     * @brief Sets blending mode.
     * @param blending new blending mode
     */
    static void setBlending(Blending blending)
    {
        ourRenderer->setBlending(blending);
    }
    /**
     * @brief Sets the window to render on.
     * @param window target window
     */
    static void setWindow(ABaseWindow* window)
    {
        ourRenderer->setWindow(window);
    }

    static glm::mat4 getProjectionMatrix() {
        return ourRenderer->getProjectionMatrix();
    }

    static const glm::mat4& getTransform()
    {
        return ourRenderer->getTransform();
    }
};

