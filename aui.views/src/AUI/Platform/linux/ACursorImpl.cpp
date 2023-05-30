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
#include "AUI/Traits/callables.h"
#include "AUI/Image/IDrawable.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Util/ACleanup.h"
#include <X11/X.h>
#include <X11/cursorfont.h>

struct ACursor::Custom {
public:
    Custom(const AImage& img) {
    }

    ~Custom() {
    }

private:
};

namespace {
    class NativeCursorHandle: public aui::noncopyable{
    public:
        NativeCursorHandle(Cursor c): mHandle(c) {

        }

        ~NativeCursorHandle() {
            XFreeCursor(CommonRenderingContext::ourDisplay, mHandle);
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

        XDefineCursor(CommonRenderingContext::ourDisplay, window->getNativeHandle(), cursorHandle);
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
            it = nativeCursors.emplace(cursor, XCreateFontCursor(CommonRenderingContext::ourDisplay, cursor)).first;
        }
        setCursor(window, it->second.handle());
    }
}

ACursor::ACursor(aui::no_escape<AImage> image, int size) : mValue(std::make_unique<ACursor::Custom>(*image)), mSize(size) {}

void ACursor::applyNativeCursor(AWindow* pWindow) const {

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
                }
            },
            [](const _<Custom>& custom) {
                //SetCursor(custom->cursor());
            },
            [&](const _<IDrawable>& drawable) {
                static AMap<_<IDrawable>, AMap<int, _<Custom>>> cache;

                auto custom = cache[drawable].getOrInsert(int(pWindow->getDpiRatio() * 10), [&] {
                    return _new<Custom>(drawable->rasterize(glm::ivec2(mSize * pWindow->getDpiRatio())));
                });

                //SetCursor(custom->cursor());
            }
    }, mValue);
}
