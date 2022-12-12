// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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


#include <variant>
#include <AUI/Image/AImage.h>
#include "AUI/Image/IDrawable.h"

class AWindow;

/**
 * @brief Represents cursor type.
 * @ingroup views
 * @ingroup ass
 */
class API_AUI_VIEWS ACursor
{
private:
    struct Custom;

public:
    enum System {
        /**
         * Default arrow
         */
        DEFAULT,

        /**
         * Pointing finger
         */
        POINTER,

        /**
         * 'I' beam
         */
        TEXT,
    };

    ACursor(System systemCursor): mValue(systemCursor) {}
    explicit ACursor(aui::no_escape<AImage> image, int size = 16);
    explicit ACursor(_<IDrawable> drawable, int size = 16);
    explicit ACursor(AUrl imageUrl, int size = 16);

    ~ACursor();

    void applyNativeCursor(AWindow* pWindow);

private:
    std::variant<System, _<Custom>, _<IDrawable>> mValue;
    int mSize;
};
