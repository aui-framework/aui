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

#include "ACursor.h"
#include "AUI/Platform/win32/Win32Util.h"
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"

struct ACursor::Custom {
public:
    Custom(const AImage& img) {
        const auto color = aui::win32::imageRgbToBitmap(img, aui::win32::BitmapMode::RGB);

        AImage white(32, 32, AImageFormat::RGBA | AImageFormat::BYTE);
        white.fillColor({0, 0, 0, 255});

        const auto mask = aui::win32::imageRgbToBitmap(white, aui::win32::BitmapMode::A);

        ICONINFO iconinfo       = {0};
        iconinfo.fIcon          = false;
        iconinfo.xHotspot       = 0;
        iconinfo.yHotspot       = 0;
        iconinfo.hbmMask        = mask;
        iconinfo.hbmColor       = color;

        mCursor = ::CreateIconIndirect(&iconinfo);
    }

    ~Custom() {
        DestroyIcon(mCursor);
    }

    HCURSOR cursor() const noexcept {
        return mCursor;
    }

private:
    HCURSOR mCursor;
};

ACursor::ACursor(aui::no_escape<AImage> image) : mValue(std::make_unique<ACursor::Custom>(*image)) {}

ACursor::~ACursor() {

}


ACursor::ACursor(const AUrl& imageUrl): ACursor(IDrawable::fromUrl(imageUrl)->rasterize({32, 32})) {

}

void ACursor::applyNativeCursor(AWindow* pWindow) {
    std::visit(aui::lambda_overloaded {
        [](System s) {
            switch (s) {
                case ACursor::DEFAULT: {
                    static auto cursor = LoadCursor(nullptr, IDC_ARROW);
                    SetCursor(cursor);
                    break;
                }
                case ACursor::POINTER: {
                    static auto cursor = LoadCursor(nullptr, IDC_HAND);
                    SetCursor(cursor);
                    break;
                }
                case ACursor::TEXT: {
                    static auto cursor = LoadCursor(nullptr, IDC_IBEAM);
                    SetCursor(cursor);
                    break;
                }
            }
        },
        [](const _<Custom>& custom) {
            SetCursor(custom->cursor());
        }
    }, mValue);
}
