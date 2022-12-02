// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Util/AArrayView.h>
#include "IRenderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class API_AUI_VIEWS Render
{
private:
    static _unique<IRenderer> ourRenderer;

public:
    using PrerenderedString = _<IRenderer::IPrerenderedString>;
    using Texture = _<ITexture>;

    static void setRenderer(_unique<IRenderer> renderer);

    static const _unique<IRenderer>& getRenderer() {
        return ourRenderer;
    }

    /**
     * Canvas for batching multiple <code>prerender</code> string calls.
     * @return a new instance of <code>IMultiStringCanvas</code>
     */
    static _<IRenderer::IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& fontStyle) {
        return ourRenderer->newMultiStringCanvas(fontStyle);
    }

    static Texture getNewTexture() {
        return ourRenderer->getNewTexture();
    }

    /**
     * Draws simple rectangle.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     */
    static void rect(const ABrush& brush,
                     const glm::vec2& position,
                     const glm::vec2& size) {
        ourRenderer->drawRect(brush, position, size);
    }


    /**
     * Draws rounded rect (without antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    static void roundedRect(const ABrush& brush,
                            const glm::vec2& position,
                            const glm::vec2& size,
                            float radius) {
        ourRenderer->drawRoundedRect(brush, position, size, radius);
    }

    /**
     * Draws rounded rect (with antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    static void roundedRectAntialiased(const ABrush& brush,
                                       const glm::vec2& position,
                                       const glm::vec2& size,
                                       float radius) {
        ourRenderer->drawRoundedRectAntialiased(brush, position, size, radius);
    }

    /**
     * Draws rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param lineWidth border line width (px)
     */
    static void rectBorder(const ABrush& brush,
                           const glm::vec2& position,
                           const glm::vec2& size,
                           float lineWidth = 1.f) {
        ourRenderer->drawRectBorder(brush, position, size, lineWidth);
    }

    /**
     * Draws rectangle's border (with antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     * @param borderWidth border line width (px)
     */
    static void rectBorder(const ABrush& brush,
                           const glm::vec2& position,
                           const glm::vec2& size,
                           float radius,
                           int borderWidth) {
        ourRenderer->drawRectBorder(brush, position, size, radius, borderWidth);
    }

    /**
     * Draws a line between <code>p1</code> and <code>p2</code>.
     * @param brush brush
     * @param p1 first point
     * @param p2 second point
     *
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>if you want to drawElements multiple lines, consider using <code>Render::lines</code> function instead.</dd>
     * </dl>
     */
    static void line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2) {
        ourRenderer->drawLine(brush, p1, p2);
    }


    /**
     * Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     */
    static void lines(const ABrush& brush, AArrayView<glm::vec2> points) {
        ourRenderer->drawLines(brush, points);
    }

    /**
     * Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     */
    static void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points) {
        ourRenderer->drawLines(brush, points);
    }


    /**
     * Draws a rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param color shadow color
     */
    static void boxShadow(const glm::vec2& position,
                          const glm::vec2& size,
                          float blurRadius,
                          const AColor& color) {
        ourRenderer->drawBoxShadow(position, size, blurRadius, color);
    }


    /**
     * Draws string.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>
     *         This function is dramatically inefficient since it does symbol lookup for every character is the
     *         <code>string</code> and does GPU buffer allocations. If you want to render the same string for several
     *         times (frames), consider using the <a href="IRenderer::prerenderString">IRenderer::prerenderString</a>
     *         function instead.
     *     </dd>
     * </dl>
     * @param position string's top left corner position
     * @param string string to render
     * @param fs font style (optional)
     */
    static void string(const glm::vec2& position,
                       const AString& string,
                       const AFontStyle& fs = {}) {
        ourRenderer->drawString(position, string, fs);
    }

    /**
     * Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    static _<IRenderer::IPrerenderedString> prerenderString(const glm::vec2& position, const AString& text, AFontStyle& fs) {
        assert(("empty string could not be prerendered" && !text.empty()));
        return ourRenderer->prerenderString(position, text, fs);
    }


    /**
     * Sets the color which is multiplied with any brush.
     * @param color color
     */
    static void setColorForced(const AColor& color)
    {
        ourRenderer->setColorForced(color);
    }

    /**
     * Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
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
     * Sets the transform matrix which is applicable for any figure. Unlike <code>setTransformForced</code>, the new
     * matrix is multiplied by the previous matrix.
     * @param transform transform matrix
     */
    static void setTransform(const glm::mat4& transform)
    {
        ourRenderer->setTransform(transform);
    }

    /**
     * Sets the transform matrix which is applicable for any figure.
     * @param transform transform matrix
     */
    static void setTransformForced(const glm::mat4& transform)
    {
        ourRenderer->setTransformForced(transform);
    }

    /**
     * Sets blending.
     * @see <a href="Blending">Blending</a>
     */
    static void setBlending(Blending blending)
    {
        ourRenderer->setBlending(blending);
    }

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

    static void translate(const glm::vec2& offset) {
        ourRenderer->setTransformForced(glm::translate(getTransform(), glm::vec3(offset, 0.f)));
    }

    /**
     * Rotates matrix along the specified axis.
     * @param axis axis
     * @param angle angle to rotate (rad) -2pi;2pi
     */
    static void rotate(const glm::vec3& axis, float angle) {
        ourRenderer->setTransformForced(glm::rotate(getTransform(), angle, axis));
    }

    /**
     * Rotates matrix along Z axis.
     * @param angle angle to rotate (rad) -2pi;2pi
     */
    static void rotate(float angle) {
        rotate({0.f, 0.f, 1.f}, angle);
    }
};

