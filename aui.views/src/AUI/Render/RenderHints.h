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
        CanvasOrRenderer(IRenderer& renderer) : mVal(&renderer) {}
        CanvasOrRenderer(ACanvas& canvas) : mVal(&canvas) {}

        glm::mat4 getTransform() const {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) return (*r)->getTransform();
            return std::get<ACanvas*>(mVal)->getTransform();
        }
        void setTransformForced(const glm::mat4& t) {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->setTransformForced(t);
            else std::get<ACanvas*>(mVal)->setTransformForced(t);
        }
        AColor getColor() const {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) return (*r)->getColor();
            return std::get<ACanvas*>(mVal)->getColor();
        }
        void setColorForced(const AColor& c) {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->setColorForced(c);
            else std::get<ACanvas*>(mVal)->setColorForced(c);
        }
        void pushMaskBefore() {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->pushMaskBefore();
            else std::get<ACanvas*>(mVal)->pushMaskBefore();
        }
        void pushMaskAfter() {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->pushMaskAfter();
            else std::get<ACanvas*>(mVal)->pushMaskAfter();
        }
        void popMaskBefore() {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->popMaskBefore();
            else std::get<ACanvas*>(mVal)->popMaskBefore();
        }
        void popMaskAfter() {
            if (auto* r = std::get_if<IRenderer*>(&mVal)) (*r)->popMaskAfter();
            else std::get<ACanvas*>(mVal)->popMaskAfter();
        }
    private:
        std::variant<IRenderer*, ACanvas*> mVal;
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
