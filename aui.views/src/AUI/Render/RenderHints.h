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
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include "IRenderer.h"

namespace RenderHints {
    /**
     * @brief Increases mask stack. Used by AView.
     * @param render renderer
     * @param maskRenderer function - mask renderer
     * @details
     * This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     * wrapper class instead.
     */
    template<aui::invocable Callable>
    static void pushMask(IRenderer& render, Callable&& maskRenderer) {
        render.pushMaskBefore();
        maskRenderer();
        render.pushMaskAfter();
    }

    /**
     * @brief Decreases mask stack. Used by AView.
     * @param render renderer
     * @param maskRenderer function - mask renderer
     * @details
     * This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
     * wrapper class instead.
     */
    template<aui::invocable Callable>
    static void popMask(IRenderer& render, Callable&& maskRenderer) {
        render.popMaskBefore();
        maskRenderer();
        render.popMaskAfter();
    }

    template<aui::invocable Callable>
    struct PushMask {
    public:
        inline explicit PushMask(IRenderer& render, Callable&& maskRenderer) :
                render(render),
                maskRenderer(std::forward<Callable>(maskRenderer)) {
            pushMask(render, std::forward<Callable>(maskRenderer));
        }

        inline ~PushMask() {
            popMask(render, std::forward<Callable>(maskRenderer));
        }

    private:
        IRenderer& render;
        Callable maskRenderer;
    };

    struct PushMatrix {
    public:
        inline explicit PushMatrix(IRenderer& render) : render(render), stored(render.getTransform()) {
        }

        inline ~PushMatrix() {
            render.setTransformForced(stored);
        }

    private:
        IRenderer& render;
        glm::mat4 stored;
    };

    struct PushColor {
    public:
        inline explicit PushColor(IRenderer& render): render(render), stored(render.getColor()) {
        }

        inline ~PushColor() {
            render.setColorForced(stored);
        }

    private:
        IRenderer& render;
        AColor stored;
    };

    class PushState : PushColor, PushMatrix {
    public:
        inline explicit PushState(IRenderer& render): PushColor(render), PushMatrix(render) {}
    };
}
