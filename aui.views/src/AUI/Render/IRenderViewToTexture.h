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
#include <AUI/Common/AStaticVector.h>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Render/ARenderContext.h>

class IRenderer;
class AView;

/**
 * @brief Rendering view to texture storage interface.
 * @see IRenderer::newRenderViewToTexture
 */
class API_AUI_VIEWS IRenderViewToTexture {
public:

    /**
     * @brief Defines areas to invalidate (redraw).
     */
    struct InvalidArea {
        /**
         * @brief No redraw.
         */
        struct Empty {};

        /**
         * @brief Full redraw.
         */
        struct Full {};

        /**
         * @brief Specific areas redraw.
         */
        using Rectangles = AStaticVector<ARect<int>, ARenderContext::Rectangles::capacity() - 1>;

        using Underlying = std::variant<Empty, Rectangles, Full>;

        template<aui::convertible_to<Underlying> F>
        InvalidArea(F&& u) noexcept: mUnderlying(std::forward<F>(u)) {}
        InvalidArea() noexcept: mUnderlying(Empty{}) {}


        [[nodiscard]]
        bool empty() const noexcept {
            return std::holds_alternative<Empty>(mUnderlying);
        }

        [[nodiscard]]
        bool full() const noexcept {
            return std::holds_alternative<Full>(mUnderlying);
        }

        /**
         * @brief Adds rectangle as an invalid area. If needed, switches to full redraw.
         */
        void addRectangle(ARect<int> rhs);

        Rectangles* rectangles() noexcept {
            return std::get_if<Rectangles>(&mUnderlying);
        }

        const Rectangles* rectangles() const noexcept {
            return std::get_if<Rectangles>(&mUnderlying);
        }

    private:
        Underlying mUnderlying;
    };




    /**
     * @brief Instructs the renderer to begin drawing to the surface (framebuffer) stored in IRenderViewToTexture.
     *
     * @param renderer renderer to draw with. IRenderViewToTexture is expected to be associated with the renderer
     * it's created with and normally this parameter is used to assert check the used renderer is the same.
     * @param surfaceSize framebuffer size. Adjusts this value to achieve supersampling. Resizes the surface if
     * mismatched with surfaceSize.
     * @param invalidArea invalid areas to update. Should not be empty. Can be changed to InvalidArea::Full{} by
     * implementation and caller is expected to react accordingly.
     * @return true on success, false otherwise
     *
     * @details
     * If needed, implementation may adjust renderer's transform matrix.
     */
    virtual bool begin(IRenderer& renderer, glm::ivec2 surfaceSize, IRenderViewToTexture::InvalidArea& invalidArea) = 0;

    /**
     * @brief Finishes drawing operation started with begin method.
     * @param renderer renderer to draw with. IRenderViewToTexture is expected to be associated with the renderer
     * it's created with and normally this parameter is used to assert check the used renderer is the same.
     * @details
     * The caller is obligated to recover renderer's state prior to begin method call.
     */
    virtual void end(IRenderer& renderer) = 0;

    /**
     * @brief Draws contents of the surface.
     * @param renderer renderer to draw with. IRenderViewToTexture is expected to be associated with the renderer
     * it's created with and normally this parameter is used to assert check the used renderer is the same.
     */
    virtual void draw(IRenderer& renderer) = 0;

    virtual ~IRenderViewToTexture() = default;

    static void enableForView(IRenderer& renderer, AView& view);
    static void disableForView(AView& view);

    [[nodiscard]]
    static bool isEnabledForView(AView& view);
};
