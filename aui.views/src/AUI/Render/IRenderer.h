#pragma once

#include <glm/glm.hpp>
#include <AUI/Reflect/AEnumerate.h>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/ASide.h>
#include <AUI/Common/AColor.h>
#include <AUI/Render/ABrush.h>
#include <AUI/Util/APool.h>
#include "AUI/Font/AFontStyle.h"
#include "ITexture.h"
#include "ATextLayoutHelper.h"

class AColor;
class ABaseWindow;



/**
 * Blending mode (like in Photoshop)
 */
enum class Blending {
    /**
     * Normal blending.
     */
    NORMAL,

    /**
     * <code>1 - COLOR</code>
     */
    INVERSE
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
    public:
        virtual ~IMultiStringCanvas() = default;

        /**
         * Bake string with some position.
         * @param position position
         * @param text text
         */
        virtual void addString(const glm::vec2& position, const AString& text) = 0;

        /**
         * @return instance of <code>Render::PrerenderedString</code> to draw with.
         */
        virtual _<IRenderer::IPrerenderedString> build() = 0;

        /**
         * @return an instance of <code>IRenderer::ITextLayoutHelper</code> constructed from
         * <code>IMultiStringCanvas</code>'s cache to efficiently map cursor position to the string index.
         */
        virtual ATextLayoutHelper makeTextLayoutHelper() = 0;
    };

protected:
    AColor mColor;
    glm::mat4 mTransform;
    ABaseWindow* mWindow = nullptr;
    APool<ITexture> mTexturePool;

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
    virtual _<IMultiStringCanvas> newMultiStringCanvas(const AFontStyle style) = 0;

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
     * @param position string's top left corner position
     * @param string string to render
     * @param fs font style (optional)
     */
    virtual void drawString(const glm::vec2& position,
                            const AString& string,
                            const AFontStyle& fs = {}) = 0;

    /**
     * Analyzes string and creates an instance of <code>IRenderer::IPrerenderedString</code> which helps
     * <code>IRenderer</code> to efficiently render the string.
     * @param text string to prerender
     * @param fs font style
     * @return an instance of IPrerenderedString
     */
    virtual _<IPrerenderedString> prerenderString(const glm::vec2& position, const AString& text, const AFontStyle& fs) = 0;


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

    virtual void setBlending(Blending blending) = 0;

    virtual void setWindow(ABaseWindow* window)
    {
        mWindow = window;
        setColorForced(1.f);
        setTransformForced(getProjectionMatrix());
    }

    glm::mat4 getProjectionMatrix() const;

    const glm::mat4& getTransform()
    {
        return mTransform;
    }
};


