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
#include <variant>
#include <AUI/Views.h>
#include <AUI/Common/AColor.h>
#include <AUI/Render/IRenderer.h>
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
    private:
        ACanvas& mCanvas;
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

    class PushState {
    public:
        inline explicit PushState(IRenderer& render): mCanvas(render.canvas()), mSaved(mCanvas.save()) {}
        inline explicit PushState(ACanvas& canvas): mCanvas(canvas), mSaved(mCanvas.save()) {}

        inline ~PushState() {
            mCanvas.restore(mSaved);
        }

    private:
        ACanvas& mCanvas;
        size_t mSaved;
    };
}
