// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
