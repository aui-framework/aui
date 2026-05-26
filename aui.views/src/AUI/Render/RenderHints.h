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
#include <variant>
#include <AUI/Render/ACanvas.hpp>

#include <AUI/Render/IRendererBackend.h>

namespace RenderHints {
    class CanvasOrRenderer {
    public:
        CanvasOrRenderer(IRenderer& renderer) : mCanvas(renderer.canvas()) {}
        CanvasOrRenderer(ACanvas& canvas) : mCanvas(canvas) {}

        glm::mat4 getTransform() const {
            return mCanvas.getTransform();
        }
        void setTransformForced(const glm::mat4& t) {
            mCanvas.setTransformForced(t);
        }
        AColor getColor() const {
            return mCanvas.getColor();
        }
        void setColorForced(const AColor& c) {
            mCanvas.setColorForced(c);
        }
        void pushMaskBefore() {
            mCanvas.pushMaskBefore();
        }
        void pushMaskAfter() {
            mCanvas.pushMaskAfter();
        }
        void popMaskBefore() {
            mCanvas.popMaskBefore();
        }
        void popMaskAfter() {
            mCanvas.popMaskAfter();
        }
    private:
        ACanvas& mCanvas;
    };

    template<aui::invocable Callable>
    static void pushMask(CanvasOrRenderer render, Callable&& maskRenderer) {
        render.pushMaskBefore();
        maskRenderer();
        render.pushMaskAfter();
    }

    template<aui::invocable Callable>
    static void popMask(CanvasOrRenderer render, Callable&& maskRenderer) {
        render.popMaskBefore();
        maskRenderer();
        render.popMaskAfter();
    }

    template<aui::invocable Callable>
    struct PushMask {
    public:
        inline explicit PushMask(CanvasOrRenderer render, Callable&& maskRenderer) :
                render(render),
                maskRenderer(std::forward<Callable>(maskRenderer)) {
            pushMask(render, std::forward<Callable>(maskRenderer));
        }

        inline ~PushMask() {
            popMask(render, std::forward<Callable>(maskRenderer));
        }

    private:
        CanvasOrRenderer render;
        Callable maskRenderer;
    };

    struct PushMatrix {
    public:
        inline explicit PushMatrix(CanvasOrRenderer render) : render(render), stored(render.getTransform()) {
        }

        inline ~PushMatrix() {
            render.setTransformForced(stored);
        }

    private:
        CanvasOrRenderer render;
        glm::mat4 stored;
    };

    struct PushColor {
    public:
        inline explicit PushColor(CanvasOrRenderer render): render(render), stored(render.getColor()) {
        }

        inline ~PushColor() {
            render.setColorForced(stored);
        }

    private:
        CanvasOrRenderer render;
        AColor stored;
    };

    class PushState : PushColor, PushMatrix {
    public:
        inline explicit PushState(CanvasOrRenderer render): PushColor(render), PushMatrix(render) {}
    };
}
