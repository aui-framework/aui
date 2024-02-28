// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AUI/Image/AImage.h"
#include <Windows.h>

namespace aui::win32 {
    API_AUI_VIEWS AImage iconToImage(HICON hIcon);
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


