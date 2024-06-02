// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <AUI/Common/AColor.h>
#include <AUI/GL/gl.h>
#include <AUI/Views.h>

#include <glm/glm.hpp>

#include "ARender.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Util/Assert.h"
#include "AUI/Util/kAUI.h"

namespace RenderHints {
/**
 * @brief Increases mask stack. Used by AView.
 * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
 *       wrapper class instead.
 * @param maskRenderer function - mask renderer
 */
template <aui::invocable Callable>
static void pushMask(Callable&& maskRenderer, IRenderer& renderer) {
    renderer.pushMaskBefore();
    AUI_DEFER { renderer.pushMaskAfter(); };
    maskRenderer();
}

/**
 * @brief Decreases mask stack. Used by AView.
 * @note This function is unsafe. It is faster, simpler and safer to use the <code>RenderHints::PushMask</code>
 *       wrapper class instead.
 * @param maskRenderer function - mask renderer
 */
template <aui::invocable Callable>
static void popMask(Callable&& maskRenderer, IRenderer& renderer) {
    renderer.popMaskBefore();
    AUI_DEFER { renderer.pushMaskAfter(); };
    maskRenderer();
}

template <aui::invocable Callable>
class PushMask {
   private:
    Callable mMaskRenderer;
    IRenderer& mRenderer;

   public:
    inline explicit PushMask(Callable&& maskRenderer, IRenderer& renderer = *ARender::getRenderer())
        : mMaskRenderer(std::forward<Callable>(maskRenderer)), mRenderer(renderer) {
        pushMask(std::forward<Callable>(mMaskRenderer), renderer);
    }
    inline ~PushMask() { popMask(std::forward<Callable>(mMaskRenderer), mRenderer); }
};

class PushMatrix {
   private:
    glm::mat4 mStored;
    IRenderer& mRenderer;

   public:
    inline PushMatrix(IRenderer& renderer = *ARender::getRenderer()) : mRenderer(renderer) {
        mStored = mRenderer.getTransform();
    }

    inline ~PushMatrix() { mRenderer.setTransformForced(mStored); }
};
class PushColor {
   private:
    AColor mStored;
    IRenderer& mRenderer;

   public:
    inline PushColor(IRenderer& renderer = *ARender::getRenderer()) : mRenderer(renderer) {
        mStored = mRenderer.getColor();
    }
    inline ~PushColor() { mRenderer.setColorForced(mStored); }
};
class PushState : PushColor, PushMatrix {
   public:
    inline PushState(IRenderer& renderer = *ARender::getRenderer()) : PushColor(renderer), PushMatrix(renderer) {}
};
};   // namespace RenderHints
