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


struct CursorCustomImpl: ACursor::Custom {
public:
    CursorCustomImpl(AImageView img, glm::ivec2 hotspot) : mCursor(aui::win32::imageToCursor(img, hotspot)) {}

    HCURSOR cursor() const noexcept {
        return mCursor.get();
    }

private:
    aui::win32::Icon mCursor;
};

ACursor::ACursor(AImageView image, Properties properties)
  : mValue(std::make_unique<CursorCustomImpl>(image, hotspotInPixels(properties.hotspot, image.size()))),
    mSize(properties.size),
    mHotspot(properties.hotspot) {}

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
                    case ACursor::NWSE_RESIZE: {
                        static auto cursor = LoadCursor(nullptr, IDC_SIZENWSE);
                        SetCursor(cursor);
                        break;
                    }
                    case ACursor::NESW_RESIZE: {
                        static auto cursor = LoadCursor(nullptr, IDC_SIZENESW);
                        SetCursor(cursor);
                        break;
                    }
                }
            },
            [](const _<Custom>& custom) {
                if (auto impl = dynamic_cast<CursorCustomImpl*>(custom.get())) {
                    SetCursor(impl->cursor());
                }
            },
            [&](const _<IDrawable>& drawable) {
                static AMap<_<IDrawable>, AMap<std::tuple<int, int, float, float>, _<CursorCustomImpl>>> cache;

                auto custom = cache[drawable].getOrInsert(
                    { int(pWindow->getDpiRatio() * 10), mSize, mHotspot.x, mHotspot.y }, [&] {
                        auto image = drawable->rasterize(glm::ivec2(mSize * pWindow->getDpiRatio()));
                        return _new<CursorCustomImpl>(image, hotspotInPixels(mHotspot, image.size()));
                    });

                SetCursor(custom->cursor());
            }
    }, mValue);
}
