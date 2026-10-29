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

        /**
         * @brief Bidirectional resize cursor northwest-southeast.
         */
        NWSE_RESIZE,

        /**
         * @brief Bidirectional resize cursor northeast-southwest.
         */
        NESW_RESIZE,
    };

    /**
     * @brief Well-known values for `Properties::hotspot`.
     */
    struct Hotspot {
        static constexpr glm::vec2 TOP_LEFT { 0.f, 0.f };
        static constexpr glm::vec2 CENTER { 0.5f, 0.5f };
    };

    /**
     * @brief Parameters of a custom (image) cursor.
     * @details
     * ```cpp
     * setCursor(ACursor(drawable, { .size = 32, .hotspot = ACursor::Hotspot::CENTER }));
     * ```
     */
    struct Properties {
        /**
         * @brief Size of the cursor, in dips.
         */
        int size = 16;

        /**
         * @brief The active point of the cursor, relative to its image: `{0, 0}` is the top left corner (the tip of
         * an arrow), `{0.5, 0.5}` is the center (a crosshair or a circle), `{1, 1}` is the bottom right corner.
         * @details
         * Relative, so it does not depend on the size of the cursor or the display scaling.
         */
        glm::vec2 hotspot = Hotspot::TOP_LEFT;
    };

    ACursor(System systemCursor): mValue(systemCursor) {}
    // a default argument `Properties = {}` can't be used here: Properties is not complete yet at this point.
    explicit ACursor(AImageView image) : ACursor(std::move(image), Properties {}) {}
    explicit ACursor(AImageView image, Properties properties);
    explicit ACursor(aui::non_null<_<IDrawable>> drawable) : ACursor(std::move(drawable), Properties {}) {}
    explicit ACursor(aui::non_null<_<IDrawable>> drawable, Properties properties);
    explicit ACursor(AUrl imageUrl) : ACursor(std::move(imageUrl), Properties {}) {}
    explicit ACursor(AUrl imageUrl, Properties properties);

    /**
     * @brief Shortcuts for a cursor with the top left hotspot of the specified size.
     */
    explicit ACursor(AImageView image, int size) : ACursor(std::move(image), Properties { .size = size }) {}
    explicit ACursor(aui::non_null<_<IDrawable>> drawable, int size)
      : ACursor(std::move(drawable), Properties { .size = size }) {}
    explicit ACursor(AUrl imageUrl, int size) : ACursor(std::move(imageUrl), Properties { .size = size }) {}

    ~ACursor();

    void applyNativeCursor(AWindow* pWindow) const;

    struct Custom {
        virtual ~Custom() = default;
    };
    const std::variant<System, _<Custom>, _<IDrawable>>& value() const { return mValue; }
    int size() const { return mSize; }

    /**
     * @return `Properties::hotspot`
     */
    glm::vec2 hotspot() const { return mHotspot; }

    /**
     * @return the hotspot in pixels of an image of the specified size
     */
    static glm::ivec2 hotspotInPixels(glm::vec2 hotspot, glm::uvec2 imageSize) {
        return glm::ivec2(glm::round(hotspot * glm::vec2(imageSize)));
    }

private:
    std::variant<System, _<Custom>, _<IDrawable>> mValue;
    int mSize = 16;
    glm::vec2 mHotspot = Hotspot::TOP_LEFT;
};
