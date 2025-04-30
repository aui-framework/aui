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

#include <X11/X.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>

#include "PlatformAbstractionX11.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Util/ACleanup.h"

namespace {
class NativeCursorHandle: public aui::noncopyable {
public:
    NativeCursorHandle(Cursor c): mHandle(c) {

    }

    ~NativeCursorHandle() {
        XFreeCursor(PlatformAbstractionX11::ourDisplay, mHandle);
    }

    [[nodiscard]]
    Cursor handle() const noexcept {
        return mHandle;
    }

private:
    Cursor mHandle;
};




void setCursor(AWindow* window, Cursor cursorHandle) {
    static Cursor prevCursor = -1;
    if (prevCursor == cursorHandle) return;
    prevCursor = cursorHandle;

    XDefineCursor(PlatformAbstractionX11::ourDisplay, window->getNativeHandle(), cursorHandle);
}

void setFontCursor(AWindow* window, int cursor) {
    static std::unordered_map<int, NativeCursorHandle> nativeCursors;
    do_once {
        ACleanup::afterEntry([&] {
          nativeCursors.clear();
        });
    }

    auto it = nativeCursors.find(cursor);
    if (it == nativeCursors.end()) {
        it = nativeCursors.emplace(cursor, XCreateFontCursor(PlatformAbstractionX11::ourDisplay, cursor)).first;
    }
    setCursor(window, it->second.handle());
}
}

void PlatformAbstractionX11::applyNativeCursor(const ACursor& cursor, AWindow* pWindow) {
    if (!bool(PlatformAbstractionX11::ourDisplay)) {
        return;
    }
    std::visit(aui::lambda_overloaded {
        [&](System s) {
          switch (s) {
              // https://tronche.com/gui/x/xlib/appendix/b/
              default: {
                  setFontCursor(pWindow, XC_arrow);
                  break;
              }
              case ACursor::POINTER: {
                  setFontCursor(pWindow, XC_hand2);
                  break;
              }
              case ACursor::TEXT: {
                  setFontCursor(pWindow, XC_xterm);
                  break;
              }
              case ACursor::EW_RESIZE: {
                  setFontCursor(pWindow, XC_sb_h_double_arrow);
                  break;
              }
              case ACursor::NS_RESIZE: {
                  setFontCursor(pWindow, XC_sb_v_double_arrow);
                  break;
              }
              case ACursor::MOVE: {
                  setFontCursor(pWindow, XC_fleur);
                  break;
              }
          }
        },
        [&](const _<Custom>& custom) {
          setCursor(pWindow, custom->handle());
        },
        [&](const _<IDrawable>& drawable) {
          static AMap<_<IDrawable>, AMap<int, _<Custom>>> cache;

          auto custom = cache[drawable].getOrInsert(int(pWindow->getDpiRatio() * 10), [&] {
            return _new<Custom>(drawable->rasterize(glm::ivec2(mSize * pWindow->getDpiRatio())));
          });

          setCursor(pWindow, custom->handle());
        }
    }, mValue);
}
