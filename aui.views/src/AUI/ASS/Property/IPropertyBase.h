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

#include <AUI/api.h>
#include <AUI/Geometry2D/ARect.h>

class API_AUI_VIEWS AView;
class API_AUI_VIEWS AAssHelper;
struct ARenderContext;

namespace ass::prop {
    /**
     * @brief Defines the types of renderable properties and order of their rendering.
     */
    enum class PropertySlot {
        NONE,

        TRANSFORM_OFFSET,
        TRANSFORM_ROTATE,
        TRANSFORM_SCALE,

        SHADOW,
        RENDER_OVERFLOW,
        IMAGE_RENDERING,
        BACKDROP,
        BACKGROUND_SOLID,
        BACKGROUND_IMAGE,
        BACKGROUND_EFFECT,
        BORDER,
        SHADOW_INNER,
        TEXT_SHADOW,

        COUNT,
    };

    /**
     * @brief Base class for all properties.
     * @ingroup ass
     */
    struct API_AUI_VIEWS IPropertyBase { // ignore ass_properties
    public:
        virtual ~IPropertyBase() = default;
        virtual void applyFor(AView* view) {};
        virtual void renderFor(AView* view, const ARenderContext& ctx) {}
        virtual bool isNone() { return false; }
        [[nodiscard]] virtual PropertySlot getPropertySlot() const {
            return PropertySlot::NONE;
        }
        virtual void updateInvalidPixelRect(ARect<int>& invalidRect) const {}

    };
    template<typename PropertyStruct>
    struct Property;
}

namespace ass {
    /**
     * @brief Indicates that the value for this property should be inherited from parent view.
     * @ingroup ass
     */
    static constexpr struct inherit_t {} inherit;
}
