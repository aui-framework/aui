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

#include <glm/glm.hpp>

class IRenderer;
class AView;

/**
 * @brief Rendering view to texture storage interface.
 * @see IRenderer::newRenderViewToTexture
 */
class IRenderViewToTexture {
public:
    /**
     * @brief Instructs the renderer to begin drawing to the surface (framebuffer) stored in IRenderViewToTexture.
     *
     * @param renderer renderer to draw with. IRenderViewToTexture is expected to be associated with the renderer
     * it's created with and normally this parameter is used to assert check the used renderer is the same.
     * @param surfaceSize framebuffer size. Adjusts this value to achieve supersampling. Resizes the surface if
     * mismatched with surfaceSize.
     * @details
     * If needed, adjusts renderer's transform matrix.
     */
    virtual void begin(IRenderer& renderer, glm::ivec2 surfaceSize) = 0;

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
};
