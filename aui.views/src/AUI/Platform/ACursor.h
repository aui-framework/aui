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


#include <variant>
#include <AUI/Image/AImage.h>
#include "AUI/Image/IDrawable.h"

class AWindow;

/**
 * @brief Represents cursor type.
 * @ingroup ass_properties
 */
class API_AUI_VIEWS ACursor
{
public:
    enum System {
        /**
         * @brief Default arrow.
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_default.png">
         */
        DEFAULT,

        /**
         * @brief Pointing finger.
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_pointer.png">
         */
        POINTER,

        /**
         * @brief 'I' beam.
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_text.png">
         */
        TEXT,

        /**
         * @brief Something to be moved. Crosshair-like cursor.
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_move.png">
         */
        MOVE,

        /**
         * @brief Bidirectional resize cursor east-west. <->
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_ew_resize.png">
         */
        EW_RESIZE,

        /**
         * @brief Bidirectional resize cursor north-south.
         * @details
         * <img src="https://github.com/aui-framework/aui/raw/master/docs/imgs/cursor_ns_resize.png">
         */
        NS_RESIZE,
    };

    ACursor(System systemCursor): mValue(systemCursor) {}
    explicit ACursor(AImageView image, int size = 16);
    explicit ACursor(aui::non_null<_<IDrawable>> drawable, int size = 16);
    explicit ACursor(AUrl imageUrl, int size = 16);

    ~ACursor();

    void applyNativeCursor(AWindow* pWindow) const;

    struct Custom {
        virtual ~Custom() = default;
    };
    const std::variant<System, _<Custom>, _<IDrawable>>& value() const { return mValue; }
    int size() const { return mSize; }

private:
    std::variant<System, _<Custom>, _<IDrawable>> mValue;
    int mSize;
};
