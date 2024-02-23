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

#include <glm/glm.hpp>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/ASide.h>
#include <AUI/Common/AColor.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Util/APool.h>
#include <AUI/Util/AArrayView.h>
#include "AUI/Font/AFontStyle.h"
#include "AUI/Render/ABorderStyle.h"
#include "AUI/Util/AMetric.h"
#include "ITexture.h"
#include "ATextLayoutHelper.h"

class AColor;
class ABaseWindow;



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
 * @brief Base class for rendering (for drawing use ARender facade instead).
 * @ingroup views
 * @details
 * Renderer is shared between windows. It's expected to share resources (if any). Thus, it does not perform any platform
 * specific routines.
 *
 * @sa IRenderingContext
 */
class IRenderer {
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
         * @note should be called by the implementation of IMultiStringCanvas.
         * @param symbol symbol to add
         */
        void notifySymbolAdded(const ATextLayoutHelper::Symbol& symbol) noexcept {
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
         * @brief Bakes a string with some position.
         * @param position position
         * @param text text
         */
        virtual void addString(const glm::ivec2& position, const AString& text) noexcept = 0;

        /**
         * @brief Bakes multi string canvas to IPrerenderedString which can be used for drawing text.
         * @note invalidates IMultiStringCanvas which speeds up some implementations of IMultiStringCanvas.
         * @return instance of <code>ARender::PrerenderedString</code> to drawElements with.
         */
        virtual _<IRenderer::IPrerenderedString> finalize() noexcept = 0;

        /**
         * @brief Returns text layout helper.
         * @return an instance of <code>IRenderer::ITextLayoutHelper</code> constructed from
         * <code>IMultiStringCanvas</code>'s cache to efficiently map cursor position to the string index.
         * @details
         * @note call enableCachingForTextLayoutHelper before adding
         *       strings.
         * @note can be called only once.
         */
        ATextLayoutHelper getTextLayoutHelper() noexcept {
            assert(("call enableCachingForTextLayoutHelper() before using getTextLayoutHelper" && bool(mSymbols)));
            return ATextLayoutHelper(std::move(*mSymbols));
        }
    };

protected:
    AColor mColor;
    glm::mat4 mTransform;
    ABaseWindow* mWindow = nullptr;
    APool<ITexture> mTexturePool;
    uint8_t mStencilDepth = 0;

    virtual ITexture* createNewTexture() = 0;

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
    virtual void drawRect(const ABrush& brush,
                          glm::vec2 position,
                          glm::vec2 size) = 0;


    /**
     * @brief Draws rounded rect (with antialiasing, if msaa enabled).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    virtual void drawRoundedRect(const ABrush& brush,
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
    virtual void drawRectBorder(const ABrush& brush,
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
    virtual void drawRoundedRectBorder(const ABrush& brush,
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
    virtual void drawBoxShadow(glm::vec2 position,
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
     * @param offset shadow offset. Unlike outer shadow (ARender::boxShadow), the offset is passed to the shader instead
     *               of a simple rectangle position offset.
     */
    virtual void drawBoxShadowInner(glm::vec2 position,
                                    glm::vec2 size,
                                    float blurRadius,
                                    float spreadRadius,
                                    float borderRadius,
                                    const AColor& color,
                                    glm::vec2 offset) = 0;

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
    virtual void drawString(glm::vec2 position,
                            const AString& string,
                            const AFontStyle& fs = {}) = 0;

    /**
     * @brief Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param position string's top left point
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    virtual _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;

    /**
     * @brief Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     * @param style style
     * @param width line width
     */
    virtual void drawLines(const ABrush& brush, AArrayView<glm::vec2> points, const ABorderStyle& style, AMetric width) = 0;

    /**
     * @brief Draws points list.
     * @param brush brush
     * @param points points
     * @param size point size
     */
    virtual void drawPoints(const ABrush& brush, AArrayView<glm::vec2> points, AMetric size) = 0;

    /**
     * @brief Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     * @param style style
     * @param width line width
     */
    virtual void drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points, const ABorderStyle& style, AMetric width) = 0;

    /**
     * @brief Draws sector in rectangle shape. The sector is drawn clockwise from begin to end angles.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @details
     * The method can be used as mask to ARender::roundedRect, creating arc shape.
     */
    virtual void drawSquareSector(const ABrush& brush,
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
     * @bruef Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
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
     * @brief Sets the window to render on.
     * @param window target window
     */
    virtual void setWindow(ABaseWindow* window)
    {
        mWindow = window;
        setColorForced(1.f);
        setTransformForced(getProjectionMatrix());
        mStencilDepth = 0;
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
};


