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

#include <AUI/Platform/ACursor.h>
#include "AUI/Platform/win32/Win32Util.h"
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"


struct ACursor::Custom {
public:
    Custom(const AImage& img) {
        const auto color = aui::win32::imageRgbToBitmap(img, aui::win32::BitmapMode::RGB);

        AImage white(img.size(), APixelFormat::RGBA | APixelFormat::BYTE);
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

void ACursor::applyNativeCursor(AWindow* pWindow) const {
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
                    case ACursor::MOVE: {
                        static auto cursor = LoadCursor(nullptr, IDC_SIZEALL);
                        SetCursor(cursor);
                        break;
                    }
                    case ACursor::NS_RESIZE: {
                        static auto cursor = LoadCursor(nullptr, IDC_SIZENS);
                        SetCursor(cursor);
                        break;
                    }
                    case ACursor::EW_RESIZE: {
                        static auto cursor = LoadCursor(nullptr, IDC_SIZEWE);
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
