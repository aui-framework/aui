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
class NativeCursorHandle : public aui::noncopyable {
public:
    NativeCursorHandle(Cursor c) : mHandle(c) {}

    ~NativeCursorHandle() { XFreeCursor(PlatformAbstractionX11::ourDisplay, mHandle); }

    [[nodiscard]]
    Cursor handle() const noexcept {
        return mHandle;
    }

private:
    Cursor mHandle;
};

class CursorCustomX11 : public ACursor::Custom, public NativeCursorHandle {
public:
    CursorCustomX11(AImageView img)
      : NativeCursorHandle([&]() -> Cursor {
          auto image = XcursorImageCreate(img.width(), img.height());
          if (image == nullptr) {
              ALogger::err("XCursor") << "XcursorImageCreate failed";
              return None;
          }
          image->xhot = 0;
          image->yhot = 0;
          image->delay = 0;
          static constexpr auto X_FORMAT = APixelFormat::BGRA | APixelFormat::BYTE;
          AFormattedImageView<X_FORMAT> destination(
              AByteBufferView(reinterpret_cast<const char*>(image->pixels), img.buffer().size()), img.size());

          img.visit([&](const auto& source) {
              using source_image_t = std::decay_t<decltype(source)>;
              using destination_image_t = std::decay_t<decltype(destination)>;

              static constexpr auto sourceFormat = (APixelFormat::Value) source_image_t::FORMAT;
              static constexpr auto destinationFormat = (APixelFormat::Value) destination_image_t::FORMAT;

              std::transform(
                  source.begin(), source.end(), const_cast<destination_image_t::Color*>(destination.begin()),
                  aui::pixel_format::convert<sourceFormat, destinationFormat>);
          });

          auto cursor = XcursorImageLoadCursor(PlatformAbstractionX11::ourDisplay, image);
          XcursorImageDestroy(image);

          return cursor;
      }()) {}

    ~CursorCustomX11() {}

private:
};

void setCursor(AWindow* window, Cursor cursorHandle) {
    static Cursor prevCursor = -1;
    if (prevCursor == cursorHandle)
        return;
    prevCursor = cursorHandle;

    XDefineCursor(PlatformAbstractionX11::ourDisplay, window->getNativeHandle(), cursorHandle);
}

void setFontCursor(AWindow* window, int cursor) {
    static std::unordered_map<int, NativeCursorHandle> nativeCursors;
    AUI_DO_ONCE {
        ACleanup::afterEntry([&] { nativeCursors.clear(); });
    }

    auto it = nativeCursors.find(cursor);
    if (it == nativeCursors.end()) {
        it = nativeCursors.emplace(cursor, XCreateFontCursor(PlatformAbstractionX11::ourDisplay, cursor)).first;
    }
    setCursor(window, it->second.handle());
}
}   // namespace

_<ACursor::Custom> PlatformAbstractionX11::createCustomCursor(AImageView image) { return _new<CursorCustomX11>(image); }

void PlatformAbstractionX11::applyNativeCursor(const ACursor& cursor, AWindow* pWindow) {
    if (!bool(PlatformAbstractionX11::ourDisplay)) {
        return;
    }
    std::visit(
        aui::lambda_overloaded {
          [&](ACursor::System s) {
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
          [&](const _<ACursor::Custom>& custom) {
              if (auto impl = _cast<CursorCustomX11>(custom)) {
                  setCursor(pWindow, impl->handle());
              }
          },
          [&](const _<IDrawable>& drawable) {
              static AMap<_<IDrawable>, AMap<int, _<CursorCustomX11>>> cache;

              auto custom = cache[drawable].getOrInsert(int(pWindow->getDpiRatio() * 10), [&] {
                  return _new<CursorCustomX11>(drawable->rasterize(glm::ivec2(cursor.size() * pWindow->getDpiRatio())));
              });

              setCursor(pWindow, custom->handle());
          } },
        cursor.value());
}

void PlatformAbstractionX11::windowSetStyle(AWindow& window, WindowStyle ws) {
    if (!!(ws & (WindowStyle::SYS | WindowStyle::NO_DECORATORS))) {
        // note the struct is declared elsewhere, is here just for clarity.
        // code is from [http://tonyobryan.com/index.php?article=9][1]
        typedef struct Hints {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long inputMode;
            unsigned long status;
        } Hints;

        /*
         * flags:
         * LS
         *  0 - close disable
         *  1 - decorations disable
         * MS
         */
        Hints hints;

        // code to remove decoration
        hints.flags = 2;
        hints.decorations = 0;
        XChangeProperty(
            PlatformAbstractionX11::ourDisplay, nativeHandle(window), PlatformAbstractionX11::ourAtoms.wmHints,
            PlatformAbstractionX11::ourAtoms.wmHints, 32, PropModeReplace, (unsigned char*) &hints, 5);
    }
}
