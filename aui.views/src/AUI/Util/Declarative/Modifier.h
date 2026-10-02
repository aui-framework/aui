// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include "AUI/Common/ASmallVector.h"
#include "AUI/Render/ARenderContext.h"
#include "AUI/Geometry2D/ARect.h"

#include <utility>

class AView;

namespace ass {
/**
 * @brief A lightweight container for view modifiers.
 *
 * A modifier is a callable that receives a reference to an {@link AView} and applies
 * some change to it (e.g. set a background color, change a size, etc.).
 *
 * Modifiers are immutable – every operation returns a new instance.  They can be
 * combined using the `|` operator.  When a modifier is applied to a view, the
 * stored callables are executed in the order they were added.  Because the
 * last modifier is executed last, it naturally implements a *last‑wins*
 * strategy for properties that are set by multiple modifiers.
 */
class Modifier {
public:
    using Element = std::function<void(AView&)>;

    /**
     * @brief Render context passed to renderBehind callbacks.
     * @details
     * Extends ARenderContext with layout information about the view being drawn,
     * so that render callbacks no longer need a direct reference to AView.
     * Inspired by DrawScope in Jetpack Compose.
     */
    struct RenderCtx : ARenderContext {
        /**
         * @brief The bounding rect size of the view in its local coordinate space.
         */
        glm::uvec2 size;

        /**
         * @brief Optional pointer to the view being rendered.
         * @details
         * Provided as an escape hatch for callbacks that must access view-specific state
         * (e.g. text rendering helpers). Prefer using `size` and `render` directly.
         * May be null in unit-test / off-screen contexts.
         */
        AView* view = nullptr;
    };

    /**
     * @brief Callback type for renderBehind.
     * @details
     * Receives a RenderCtx instead of AView& so that the callback is decoupled
     * from the view hierarchy.  All geometry needed for drawing (size, etc.) is
     * available through RenderCtx::rect; the renderer is available through
     * ARenderContext::render.
     */
    using RenderCallback = std::function<void(RenderCtx ctx)>;

    Modifier() = default;
    Modifier(const Modifier& other) = default;
    Modifier(Modifier&& other) noexcept = default;
    Modifier& operator=(const Modifier& other) = default;
    Modifier& operator=(Modifier&& other) noexcept = default;

    [[nodiscard]] const auto& data() const { return mData; }

    /**
     * @brief Concatenates this modifier with another.
     * @details
     * Returns a `Modifier` representing this modifier.
     */
    Modifier& then(Element element) {
        mData.elements << std::move(element);
        return *this;
    }

    /**
     * @brief Adds a new render callback (new API, no AView& dependency).
     */
    Modifier& renderBehind(RenderCallback element) {
        mData.renderBehind << std::move(element);
        return *this;
    }



private:
    struct Data {
        ASmallVector<Element, 16> elements;
        ASmallVector<RenderCallback, 4> renderBehind;
    } mData;
};
}
