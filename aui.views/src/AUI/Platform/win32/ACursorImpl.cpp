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

#include <AUI/Platform/ACursor.h>
#include "AUI/Platform/win32/Win32Util.h"
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"


struct ACursor::Custom {
public:
    Custom(const AImage& img) {
        const auto color = aui::win32::imageRgbToBitmap(img, aui::win32::BitmapMode::RGB);

        AImage white(img.size(), AImageFormat::RGBA | AImageFormat::BYTE);
        white.fill(AColor::WHITE);

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

ACursor::ACursor(aui::no_escape<AImage> image, int size) : mValue(std::make_unique<ACursor::Custom>(*image)), mSize(size) {}

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
            },
            [&](const _<IDrawable>& drawable) {
                static AMap<_<IDrawable>, AMap<int, _<Custom>>> cache;

                auto custom = cache[drawable].getOrInsert(int(pWindow->getDpiRatio() * 10), [&] {
                    return _new<Custom>(drawable->rasterize(glm::ivec2(mSize * pWindow->getDpiRatio())));
                });

                SetCursor(custom->cursor());
            }
    }, mValue);
}
