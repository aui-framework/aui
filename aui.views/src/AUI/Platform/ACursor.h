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
    explicit ACursor(aui::non_null<_<IDrawable>> drawable, int size = 16);
    explicit ACursor(AUrl imageUrl, int size = 16);

    ~ACursor();

    void applyNativeCursor(AWindow* pWindow) const;

private:
    std::variant<System, _<Custom>, _<IDrawable>> mValue;
    int mSize;
};
