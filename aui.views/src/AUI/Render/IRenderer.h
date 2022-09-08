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
     * <p>Normal blending.</p>
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
     * <p>Simply sums <code>S</code> and <code>D</code> colors.
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
     * <p>Inverses destination color and multiplies it with the source color.
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
     * <p>Inverses source color and multiplies it with the destination color.
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
         * Notifies IMultiStringCanvas than a symbol was added used to construct a
         * <a href="#ATextLayoutHelper">ATextLayoutHelper</a>.
         * @note should be called by the implementation of <a href="#IMultiStringCanvas">IMultiStringCanvas</a>.
         * @param symbol symbol to add
         */
        void notifySymbolAdded(const ATextLayoutHelper::Symbol& symbol) noexcept {
            if (mSymbols) mSymbols->last().push_back(symbol);
        }

    public:
        virtual ~IMultiStringCanvas() = default;

        /**
         * Notifies IMultiStringCanvas that <a href="#makeTextLayoutHelper">getTextLayoutHelper</a> will be used.
         */
        void enableCachingForTextLayoutHelper() noexcept {
            mSymbols = ATextLayoutHelper::Symbols{};
            nextLine();
        }

        /**
         * When caching for text layout helper is enabled, a new line added.
         */
        void nextLine() noexcept {
            if (mSymbols) mSymbols->push_back({});
        }

        /**
         * Bake string with some position.
         * @param position position
         * @param text text
         */
        virtual void addString(const glm::ivec2& position, const AString& text) noexcept = 0;

        /**
         * @note invalidates IMultiStringCanvas which speeds up some implementations of IMultiStringCanvas.
         * @return instance of <code>Render::PrerenderedString</code> to drawElements with.
         */
        virtual _<IRenderer::IPrerenderedString> finalize() noexcept = 0;

        /**
         * @note call <a href="#enableCachingForTextLayoutHelper">enableCachingForTextLayoutHelper</a> before adding
         *       strings.
         * @note can be called only once.
         * @return an instance of <code>IRenderer::ITextLayoutHelper</code> constructed from
         * <code>IMultiStringCanvas</code>'s cache to efficiently map cursor position to the string index.
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

    _<ITexture> getNewTexture() {
        return mTexturePool.get();
    }

    /**
     * Canvas for batching multiple <code>prerender</code> string calls.
     * @return a new instance of <code>IMultiStringCanvas</code>
     */
    virtual _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle& style) = 0;

    /**
     * Draws simple rectangle.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     */
    virtual void drawRect(const ABrush& brush,
                          const glm::vec2& position,
                          const glm::vec2& size) = 0;


    /**
     * Draws rounded rect (without antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    virtual void drawRoundedRect(const ABrush& brush,
                                 const glm::vec2& position,
                                 const glm::vec2& size,
                                 float radius) = 0;

    /**
     * Draws rounded rect (with antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     */
    virtual void drawRoundedRectAntialiased(const ABrush& brush,
                                            const glm::vec2& position,
                                            const glm::vec2& size,
                                            float radius) = 0;

    /**
     * Draws rectangle's border.
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param lineWidth border line width (px)
     */
    virtual void drawRectBorder(const ABrush& brush,
                                const glm::vec2& position,
                                const glm::vec2& size,
                                float lineWidth = 1.f) = 0;
    /**
     * Draws rectangle's border (with antialiasing).
     * @param brush brush to use
     * @param position rectangle position (px)
     * @param size rectangle size (px)
     * @param radius corner radius (px)
     * @param borderWidth border line width (px)
     */
    virtual void drawRectBorder(const ABrush& brush,
                                const glm::vec2& position,
                                const glm::vec2& size,
                                float radius,
                                int borderWidth) = 0;


    /**
     * Draws a rectangle-shaped shadow.
     * @param position position
     * @param size rectangle size
     * @param blurRadius blur radius
     * @param color shadow color
     */
    virtual void drawBoxShadow(const glm::vec2& position,
                               const glm::vec2& size,
                               float blurRadius,
                               const AColor& color) = 0;


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
     * @param position string's top left point
     * @param string string to render
     * @param fs font style (optional)
     */
    virtual void drawString(const glm::vec2& position,
                            const AString& string,
                            const AFontStyle& fs = {}) = 0;

    /**
     * Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param position string's top left point
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    virtual _<IPrerenderedString> prerenderString(const glm::vec2& position, const AString& text, const AFontStyle& fs) = 0;



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
    virtual void drawLine(const ABrush& brush, glm::vec2 p1, glm::vec2 p2) = 0;

    /**
     * Draws polyline (non-loop line strip).
     * @param brush brush
     * @param points polyline points
     */
    virtual void drawLines(const ABrush& brush, AArrayView<glm::vec2> points) = 0;

    /**
     * Draws multiple individual lines in a batch.
     * @param brush brush
     * @param points line points
     */
    virtual void drawLines(const ABrush& brush, AArrayView<std::pair<glm::vec2, glm::vec2>> points) = 0;

    /**
     * Sets the color which is multiplied with any brush.
     * @param color color
     */
    void setColorForced(const AColor& color)
    {
        mColor = color;
    }

    /**
     * Sets the color which is multiplied with any brush. Unlike <code>setColorForced</code>, the new color is multiplied
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
     * Sets the transform matrix which is applicable for any figure. Unlike <code>setTransformForced</code>, the new
     * matrix is multiplied by the previous matrix.
     * @param transform transform matrix
     */
    void setTransform(const glm::mat4& transform)
    {
        mTransform *= transform;
    }

    /**
     * Sets the transform matrix which is applicable for any figure.
     * @param transform transform matrix
     */
    void setTransformForced(const glm::mat4& transform)
    {
        mTransform = transform;
    }

    /**
     * Switches drawing to the stencil buffer instead of color buffer.
     * Stencil pixel is increased by each affected pixel.
     * Should be called before the <code>pushMaskAfter</code> function.
     */
    virtual void pushMaskBefore() = 0;

    /**
     * Switches drawing to the color buffer back from the stencil. Increases stencil depth.
     * Stencil buffer should not be changed after calling this function.
     * Should be called after the <code>pushMaskBefore</code> function.
     */
    virtual void pushMaskAfter() = 0;

    /**
    * Switches drawing to the stencil buffer instead of color buffer.
    * Stencil pixel is decreased by each affected pixel.
    * Should be called before the <code>popMaskAfter</code> function.
    */
    virtual void popMaskBefore() = 0;
    /**
     * Switches drawing to the color buffer back from the stencil. Decreases stencil depth.
     * Stencil buffer should not be changed after calling this function.
     * Should be called after the <code>popMaskBefore</code> function.
     */
    virtual void popMaskAfter() = 0;

    virtual void setBlending(Blending blending) = 0;

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
};


