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

#include "AUI/Image/AImage.h"
#include <Windows.h>

namespace aui::win32 {
    /**
     * @brief HICON to AImage.
     * @ingroup windows_specific
     */
    API_AUI_VIEWS AImage iconToImage(HICON hIcon);

    /**
     * @brief HBITMAP to AImage.
     * @ingroup windows_specific
     */
    API_AUI_VIEWS AImage bitmapToImage(HBITMAP hbitmap);


    enum class BitmapMode {
        RGB,
        A
    };

    namespace detail {

        template<typename T>
        struct Win32Object: aui::noncopyable {
        public:

            Win32Object(T value) : value(value) {}
            Win32Object(Win32Object&& rhs) noexcept : value(rhs.value) {
                rhs.value = nullptr;
            }

            operator T() const noexcept {
                return value;
            }

            ~Win32Object() {}

        protected:
            T value = nullptr;
        };
    }

    struct Bitmap: detail::Win32Object<HBITMAP> {
    public:
        using detail::Win32Object<HBITMAP>::Win32Object;

        ~Bitmap() {
            if (value) {
                DeleteObject(value);
            }
        }
    };

    struct DeviceContext: detail::Win32Object<HDC> {
    public:
        using detail::Win32Object<HDC>::Win32Object;

        ~DeviceContext() {
            if (value) {
                ReleaseDC(NULL, value);
            }
        }
    };

    API_AUI_VIEWS Bitmap imageRgbToBitmap(const AImage& image, BitmapMode mode = BitmapMode::RGB);
}


