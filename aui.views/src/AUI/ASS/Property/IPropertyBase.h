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

#include <AUI/api.h>

class API_AUI_VIEWS AView;
class API_AUI_VIEWS AAssHelper;

namespace ass::prop {
    enum class PropertySlot {
        NONE,

        TRANSFORM_OFFSET,
        TRANSFORM_ROTATE,
        TRANSFORM_SCALE,

        SHADOW,
        RENDER_OVERFLOW,
        IMAGE_RENDERING,
        BACKGROUND_SOLID,
        BACKGROUND_IMAGE,
        BACKGROUND_EFFECT,
        BORDER,
        SHADOW_INNER,
        TEXT_SHADOW,

        COUNT,
    };

    struct API_AUI_VIEWS IPropertyBase {
    public:
        virtual void applyFor(AView* view) {};
        virtual void renderFor(AView* view) {};
        virtual bool isNone() { return false; }
        [[nodiscard]] virtual PropertySlot getPropertySlot() const {
            return PropertySlot::NONE;
        }
    };
    template<typename PropertyStruct>
    struct Property;
}
