/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Reflect/AEnumerate.h>
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

    static void setRenderer(_unique<IRenderer> renderer) {
        ourRenderer = std::move(renderer);
    }

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
    static void drawRect(const ABrush& brush,
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
    static void drawRoundedRect(const ABrush& brush,
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
    static void drawRoundedRectAntialiased(const ABrush& brush,
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
    static void drawRectBorder(const ABrush& brush,
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
    static void drawRectBorder(const ABrush& brush,
                               const glm::vec2& position,
                               const glm::vec2& size,
                               float radius,
                               int borderWidth) {
        ourRenderer->drawRectBorder(brush, position, size, radius, borderWidth);
    }


    /**
     * Draws a rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param color shadow color
     */
    static void drawBoxShadow(const glm::vec2& position,
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
    static void drawString(const glm::vec2& position,
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
};

