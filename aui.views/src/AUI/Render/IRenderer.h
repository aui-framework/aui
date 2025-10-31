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
#include <AUI/Render/ABrush.h>
#include <AUI/Util/APool.h>
#include <AUI/Util/AArrayView.h>
#include "AUI/Font/AFontStyle.h"
#include "AUI/Render/ABorderStyle.h"
#include "AUI/ASS/Property/Backdrop.h"
#include "AUI/Util/AMetric.h"
#include "ITexture.h"
#include "ATextLayoutHelper.h"
#include "IRenderViewToTexture.h"

class AColor;
class AWindowBase;



/**
 * Blending mode.
 * <p><b>Terminology used in this documentation</b>:</p>
 * <dl>
 *   <dt><b><u>S</u>ource color</b> (S)</dt>
 *   <dd>
 *      Source color is a color of the brush (i.e. texture color matching current position) multiplied by the current
 *      renderer color, i.e. when drawing a black rectangle onto the white canvas the source color is black.
 *   </dd>
 *
 *   <dt><b><u>D</u>estination color</b> (D)</dt>
 *   <dd>
 *      Destination color is a color of the framebuffer you're drawing to, i.e. when drawing a black
 *      rectangle onto the white canvas the destination color is white.
 *   </dd>
 *
 *   <dt><b><u>S</u>.rgb</b></dt>
 *   <dd>Source color without the alpha component.</dd>
 *
 *   <dt><b><u>S</u>.a</b></dt>
 *   <dd>Source's alpha component without the color itself.</dd>
 *
 *   <dt><b><u>D</u>.rgb</b></dt>
 *   <dd>Destination color without the alpha component.</dd>
 *
 *   <dt><b><u>D</u>.a</b></dt>
 *   <dd>Destination's alpha component without the color itself.</dd>
 *
 *   <dt><b>Alpha-based</b></dt>
 *   <dd>Alpha-based blending mode is a blending mode that uses the alpha component in it's formula.</dd>
 *
 *   <dt><b>Color-based</b></dt>
 *   <dd>Color-based blending mode is a blending mode that does not use the alpha component in it's formula.</dd>
 * </dl>
 */
enum class Blending {
    /**
     * @brief Normal blending.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb * S.a + D.rgb * (1 - S.a)</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Alpha-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S.a</code> is 0, <code>NORMAL</code> does not drawElements anything.
     *     <p>When <code>S.a</code> is 1, <code>NORMAL</code> ignores <code>D</code>.
     *   </dd>
     * </dl>
     */
    NORMAL,


    /**
     * @brief Simply sums <code>S</code> and <code>D</code> colors.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb + D.rgb</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>ADDITIVE</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>ADDITIVE</code> draws white.</p>
     *   </dd>
     * </dl>
     */
    ADDITIVE,

    /**
     * @brief Inverses destination color and multiplies it with the source color.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>S.rgb * (1 - D.rgb)</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>INVERSE_DST</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>INVERSE_DST</code> does full inverse.</p>
     *   </dd>
     * </dl>
     */
    INVERSE_DST,

    /**
     * @brief Inverses source color and multiplies it with the destination color.
     * @details
     * <dl>
     *   <dt><b>Formula</b></dt>
     *   <dd><code>(1 - S.rgb) * D.rgb</code></dd>
     *   <dt><b>Type</b></dt>
     *   <dd>Color-based</dd>
     *   <dt><b>Behaviour</b></dt>
     *   <dd>
     *     <p>When <code>S</code> is black, <code>INVERSE_SRC</code> does not drawElements anything.</p>
     *     <p>When <code>S</code> is white, <code>INVERSE_SRC</code> draws black.</p>
     *   </dd>
     * </dl>
     */
    INVERSE_SRC,
};

/**
 * @brief Base class for rendering.
 * @ingroup views
 * @details
 * Renderer is shared between windows. It's expected to share resources (if any). Thus, it does not perform any platform
 * specific routines.
 *
 * @sa IRenderingContext
 */
class IRenderer: public aui::noncopyable {
public:
    class IPrerenderedString {
    public:
        virtual void draw() = 0;
        virtual ~IPrerenderedString() = default;
        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
    };
    class IMultiStringCanvas {
    private:
        AOptional<ATextLayoutHelper::Symbols> mSymbols;

    protected:

        /**
         * @brief Notifies IMultiStringCanvas than a symbol was added used to construct a ATextLayoutHelper.
         * @details
         * @param symbol symbol data to add
         * @details
         * This method should be called by the implementation of IMultiStringCanvas.
         *
         * At the end of line, implementation must add extra symbol to mark last position.
         */
        void notifySymbolAdded(const ATextLayoutHelper::Boundary& symbol) noexcept {
            if (mSymbols) mSymbols->last().push_back(symbol);
        }

    public:
        virtual ~IMultiStringCanvas() = default;

        /**
         * @brief Notifies IMultiStringCanvas that getTextLayoutHelper() will be used.
         */
        void enableCachingForTextLayoutHelper() noexcept {
            mSymbols = ATextLayoutHelper::Symbols{};
            nextLine();
        }

        /**
         * @brief When caching for text layout helper is enabled, a new line added.
         */
        void nextLine() noexcept {
            if (mSymbols) mSymbols->push_back({});
        }

        /**
         * @brief Bakes a UTF-8 string with some position.
         * @param position position
         * @param text text
         */
        virtual void addString(const glm::ivec2& position, AStringView text) noexcept = 0;

        /**
         * @brief Bakes a UTF-32 string with some position.
         * @param position position
         * @param text text
         */
        virtual void addString(const glm::ivec2& position, std::u32string_view text) noexcept = 0;

        /**
         * @brief Bakes multi string canvas to IPrerenderedString which can be used for drawing text.
         * @return instance of <code>_<IRenderer::IPrerenderedString></code> to drawElements with.
         * @details
         * Invalidates IMultiStringCanvas which speeds up some implementations of IMultiStringCanvas.
         */
        virtual _<IRenderer::IPrerenderedString> finalize() noexcept = 0;

        /**
         * @brief Returns text layout helper.
         * @return an instance of <code>IRenderer::ITextLayoutHelper</code> constructed from
         * <code>IMultiStringCanvas</code>'s cache to efficiently map cursor position to the string index.
         * @details
         * Call enableCachingForTextLayoutHelper before adding strings.
         *
         * Can be called only once.
         */
        ATextLayoutHelper getTextLayoutHelper() noexcept {
            AUI_ASSERTX(bool(mSymbols), "call enableCachingForTextLayoutHelper() before using getTextLayoutHelper");
            return ATextLayoutHelper(std::move(*mSymbols));
        }
    };

public:
    IRenderer(): mTexturePool([this] { return createNewTexture(); }) {}
    virtual ~IRenderer() = default;

    /**
     * @brief Creates new texture (image representation optimized for GPU rendering).
     */
    _<ITexture> getNewTexture() {
        return mTexturePool.get();
    }

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
     * @brief Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param position string baseline
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    virtual _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;

    /**
    * @details
    * <dl>
    *   <dt><b>Performance note</b></dt>
    *   <dd>if you want to drawElements multiple lines, consider using <code>ARender::lines</code> function instead.</dd>
    * </dl>
    */
    void line(const ABrush& brush, glm::vec2 p1, glm::vec2 p2, const ABorderStyle& style = ABorderStyle::Solid{}, AMetric width = 1_dp) {
        glm::vec2 points[] = { p1, p2 };
        lines(brush, points, style, width);
    }

    /**
     * @brief Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     * @param style style
     * @param width line width
     */
    virtual void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) = 0;

    /**
     * @brief Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     * @param style style
     */
    void lines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style = ABorderStyle::Solid{}) {
        lines(brush, points, style, 1_dp);
    }

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
     * @brief Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     * @param style style
     */
    void lines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style = ABorderStyle::Solid{}) {
        lines(brush, points, style, 1_dp);
    }

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
     * @brief Sets the color which is multiplied with any brush.
     * @param color color
     */
    void setColorForced(const AColor& color)
    {
        mColor = color;
    }

    /**
     * @brief Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
     * by the previous color.
     * @param color color
     */
    void setColor(const AColor& color)
    {
        setColorForced(mColor * color);
    }

    const AColor& getColor() const
    {
        return mColor;
    }

    /**
     * @brief Sets the transform matrix which is applicable for any figure. Unlike <code>setTransformForced</code>, the new
     * matrix is multiplied by the previous matrix.
     * @param transform transform matrix
     */
    void setTransform(const glm::mat4& transform)
    {
        mTransform *= transform;
    }

    /**
     * @brief Sets the transform matrix which is applicable for any figure.
     * @param transform transform matrix
     */
    void setTransformForced(const glm::mat4& transform)
    {
        mTransform = transform;
    }

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
     * @brief Sets blending mode.
     * @param blending new blending mode
     */
    virtual void setBlending(Blending blending) = 0;


    /**
     * @brief Returns a new instance of IRenderViewToTexture interface associated with this renderer.
     * @return A new instance. Can return null if unsupported.
     */
    [[nodiscard]]
    virtual _unique<IRenderViewToTexture> newRenderViewToTexture() noexcept = 0;

    /**
     * @brief Sets the window to render on.
     * @param window target window
     */
    virtual void setWindow(AWindowBase* window)
    {
        mWindow = window;
        setColorForced(1.f);
        setTransformForced(getProjectionMatrix());
        mStencilDepth = 0;
    }

    [[nodiscard]]
    AWindowBase* getWindow() const noexcept {
        return mWindow;
    }

    virtual glm::mat4 getProjectionMatrix() const = 0;

    const glm::mat4& getTransform()
    {
        return mTransform;
    }

    [[nodiscard]]
    std::uint8_t getStencilDepth() const noexcept {
        return mStencilDepth;
    }

    void setStencilDepth(uint8_t stencilDepth) {
        mStencilDepth = stencilDepth;
    }


    /**
     * @brief Wrapper for setTransform applying matrix translate transformation.
     * @param offset offset in pixels to translate.
     */
    void translate(const glm::vec2& offset) {
        setTransformForced(glm::translate(getTransform(), glm::vec3(offset, 0.f)));
    }

    /**
     * @brief wrapper for setTransform applying matrix rotation along the specified axis.
     * @param axis axis
     * @param angle angle to rotate
     */
    void rotate(const glm::vec3& axis, AAngleRadians angle) {
        setTransformForced(glm::rotate(getTransform(), angle.radians(), axis));
    }

    /**
     * @brief wrapper for setTransform applying matrix rotation along z axis.
     * @param angle angle to rotate
     */
    void rotate(AAngleRadians angle) {
        rotate({0.f, 0.f, 1.f}, angle);
    }

    void setAllowRenderToTexture(bool allowRenderToTexture) {
        mAllowRenderToTexture = allowRenderToTexture;
    }

    [[nodiscard]]
    bool allowRenderToTexture() const noexcept {
        return mAllowRenderToTexture;
    }

    /**
     * @brief Draws rectangular backdrop effects.
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param backdrops array of backdrop effects. Impl might apply optimizations on using several effects at once.
     * @details
     * Implementation might draw stub (i.e., gray rectangle) instead of drawing complex backdrop effects.
     */
    void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Any> backdrops);

protected:
    AColor mColor;
    glm::mat4 mTransform;
    AWindowBase* mWindow = nullptr;
    APool<ITexture> mTexturePool;
    uint8_t mStencilDepth = 0;

    virtual _unique<ITexture> createNewTexture() = 0;

    /**
     * @brief Draws stub (i.e., gray rectangle)
     * @details
     * This can be used if implementation does not support or can't draw complex effects (i.e., blur)
     */
    void stub(glm::vec2 position, glm::vec2 size);

    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops);

private:
    bool mAllowRenderToTexture = false;

};


