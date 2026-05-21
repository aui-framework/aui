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
#include "Blending.h"
#include "ADisplayList.h"

class AColor;
class ASurface;

class IPrerenderedString {
public:
    virtual void draw() = 0;
    virtual ~IPrerenderedString() = default;
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
};

/**
 * @brief Base class for renderer.
 * @ingroup views
 * @details
 * The rendering engine provides graphics rendering capabilities for the AUI Framework's user interface system. It
 * offers both hardware-accelerated (OpenGL) and software-based rendering backends. The engine handles drawing
 * primitives, text rendering, visual effects, and maintains consistent rendering behavior across different platforms
 * and hardware capabilities.
 *
 * The renderer is shared across windows and manages its own resources such as textures, but [each window has its own
 * rendering context](awindow.md).
 *
 * ## Core Renderer Interface
 *
 * | Category | Key Methods | Purpose |
 * |----------|--------------|---------|
 * | Shape Drawing | `rectangle()`, `roundedRectangle()`, `rectangleBorder()` | Basic geometric shapes |
 * | Line Drawing | `line()`, `lines()`, `points()` | Vector graphics primitives |
 * | Text Rendering | `string()`, `prerenderString()`, `newMultiStringCanvas()` | Text output and caching |
 * | Visual Effects | `boxShadow()`, `boxShadowInner()`, `squareSector()` | Advanced visual effects |
 * | State Management | `setColor()`, `setTransform()`, `setBlending()` | Rendering context control |
 * | Masking | `pushMaskBefore()`, `pushMaskAfter()`, `popMaskBefore()`, `popMaskAfter()` | Stencil-based clipping |
 *
 * The renderer maintains internal state including:
 *   
 * - Current color multiplier (mColor)
 * - Transformation matrix (mTransform)
 * - Target window (mWindow)
 * - Stencil depth for masking (mStencilDepth)
 * - Texture pool for resource management (mTexturePool)
 *
 * ## HiDPI (High‑DPI) support
 *
 * The framework uses logical units [dp](ametric.md) for layout and drawing.  All logical values are
 * multiplied by the window pixel ratio before they reach the
 * renderer. The renderer works with physical pixels only (px).
 * 
 * This ensures that the UI appears consistent across displays with varying pixel densities.
 */
class IRenderer: public aui::noncopyable {
public:
    using IPrerenderedString = ::IPrerenderedString;
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
    virtual void rectangle(const ADisplayList::Rectangle& v, const APaint& paint) = 0;
    virtual void roundedRectangle(const ADisplayList::RoundedRectangle& v, const APaint& paint) = 0;
    virtual void rectangleBorder(const ADisplayList::RectangleBorder& v, const APaint& paint) = 0;
    virtual void roundedRectangleBorder(const ADisplayList::RoundedRectangleBorder& v, const APaint& paint) = 0;
    virtual void boxShadow(const ADisplayList::BoxShadow& v, const APaint& paint) = 0;
    virtual void boxShadowInner(const ADisplayList::BoxShadowInner& v, const APaint& paint) = 0;
    virtual void string(const ADisplayList::Text& v, const APaint& paint) = 0;
    virtual _<IPrerenderedString> prerenderString(glm::vec2 position, const AString& text, const AFontStyle& fs) = 0;
    virtual void lines(const ADisplayList::Lines& v, const APaint& paint) = 0;
    virtual void points(const ADisplayList::Points& v, const APaint& paint) = 0;
    virtual void lines(const ADisplayList::LineBatches& v, const APaint& paint) = 0;
    virtual void squareSector(const ADisplayList::SquareSector& v, const APaint& paint) = 0;
 
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
     * @details
     * **Blending Modes and Effects**
     *
     * The rendering engine supports multiple blending modes for advanced visual effects:
     *
     * <!-- aui:steal_documentation Blending -->
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
    virtual void setWindow(ASurface* window)
    {
        mWindow = window;
        setColorForced(1.f);
        setTransformForced(getProjectionMatrix());
        mStencilDepth = 0;
    }

    [[nodiscard]]
    ASurface* getWindow() const noexcept {
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
     * @brief Controls the rendering scale of images for display only.
     * Does not affect the actual visual appearance or geometry of shapes.
     * Only impacts the sharpness and clarity of rendered images on screen.
     */
    void setRenderScale(float render_scale) {
        mRenderScale = render_scale;
    }

    float getRenderScale() const noexcept {
        return mRenderScale;
    }

    virtual void backdrops(const ADisplayList::Backdrop& v, const APaint& paint);

protected:
    float mRenderScale = 1.0f;
    AColor mColor;
    glm::mat4 mTransform;
    ASurface* mWindow = nullptr;
    APool<ITexture> mTexturePool;
    uint8_t mStencilDepth = 0;

    virtual _unique<ITexture> createNewTexture() = 0;

    /**
     * @brief Draws stub (i.e., gray rectangle)
     * @details
     * This can be used if implementation does not support or can't draw complex effects (i.e., blur)
     */
    void stub(glm::vec2 position, glm::vec2 size);

    virtual void backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops);

private:
    bool mAllowRenderToTexture = false;

};


