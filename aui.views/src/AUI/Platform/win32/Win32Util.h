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

#include "AUI/Image/AImage.h"
#include <Windows.h>
#include <memory>

namespace aui::win32 {
    /**
     * @brief HICON to AImage.
     * @ingroup core
     * @details
     * @exclusivefor{windows}
     */
    API_AUI_VIEWS AImage iconToImage(HICON hIcon);

    /**
     * @brief HBITMAP to AImage.
     * @ingroup core
     * @exclusivefor{windows}
     */
    API_AUI_VIEWS AImage bitmapToImage(HBITMAP hbitmap);


    /**
     * @brief How the alpha channel is stored in a bitmap created by [imageToBitmap].
     * @exclusivefor{windows}
     */
    enum class BitmapAlpha {
        /**
         * @brief Color channels are left as is. Suitable for icons and cursors.
         */
        STRAIGHT,

        /**
         * @brief Color channels are multiplied by alpha. Required for menu item bitmaps (AlphaBlend, DWM).
         */
        PREMULTIPLIED,
    };

    namespace detail {
        struct BitmapDeleter { void operator()(HBITMAP h) const noexcept { DeleteObject(h); } };
        struct MenuDeleter { void operator()(HMENU h) const noexcept { DestroyMenu(h); } };
        struct IconDeleter { void operator()(HICON h) const noexcept { DestroyIcon(h); } };
        struct DcDeleter { void operator()(HDC h) const noexcept { DeleteDC(h); } };

        template<typename Handle, typename Deleter>
        using Owned = std::unique_ptr<std::remove_pointer_t<Handle>, Deleter>;
    }

    /**
     * @brief Owning HBITMAP.
     * @exclusivefor{windows}
     */
    using Bitmap = detail::Owned<HBITMAP, detail::BitmapDeleter>;

    /**
     * @brief Owning HMENU. Destroying a menu also destroys the submenus attached to it, so release() a submenu
     * once it is appended to its parent.
     * @exclusivefor{windows}
     */
    using Menu = detail::Owned<HMENU, detail::MenuDeleter>;

    /**
     * @brief Owning HICON. HCURSOR is the same handle type, so it is used for cursors as well.
     * @exclusivefor{windows}
     */
    using Icon = detail::Owned<HICON, detail::IconDeleter>;

    /**
     * @brief Owning memory HDC (CreateCompatibleDC).
     * @exclusivefor{windows}
     */
    using MemoryDc = detail::Owned<HDC, detail::DcDeleter>;

    /**
     * @brief AImage to 32-bit top-down BGRA HBITMAP (DIB section).
     * @ingroup core
     * @exclusivefor{windows}
     */
    API_AUI_VIEWS Bitmap imageToBitmap(AImageView image, BitmapAlpha alpha = BitmapAlpha::STRAIGHT);

    /**
     * @brief AImage to icon.
     * @ingroup core
     * @exclusivefor{windows}
     */
    API_AUI_VIEWS Icon imageToIcon(AImageView image);

    /**
     * @brief AImage to cursor.
     * @ingroup core
     * @param hotspot cursor's active point, in image pixels.
     * @exclusivefor{windows}
     */
    API_AUI_VIEWS Icon imageToCursor(AImageView image, glm::ivec2 hotspot = {0, 0});
}
