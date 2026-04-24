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

#include "Win32MenuProvider.h"
#include <AUI/Action/AMenu.h>
#include <AUI/Action/AShortcut.h>
#include <AUI/Image/IDrawable.h>
#include <AUI/Image/AImage.h>
#include <AUI/Platform/AWindow.h>

#include <windows.h>

#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

namespace {

// Ids start at 1 — ID 0 from TrackPopupMenuEx means "cancelled".
constexpr UINT kFirstMenuId = 1;

// Owned resources for one popup lifetime: id→callback map + accumulated HBITMAPs
// (for icons) + HMENUs (root + all submenus), all destroyed after the modal call returns.
struct MenuResources {
    std::unordered_map<UINT, std::function<void()>> callbacks;
    std::vector<HBITMAP> bitmaps;
    std::vector<HMENU> menus;
    UINT nextId = kFirstMenuId;

    ~MenuResources() {
        for (HBITMAP hbmp : bitmaps) DeleteObject(hbmp);
        // The root HMENU destroys its own submenus cascade-style; we only need to
        // destroy root menus (tracked as the first entry in `menus`).
        if (!menus.empty()) DestroyMenu(menus.front());
    }
};

std::wstring toWide(const AString& s) {
    const std::string utf8 = s.toStdString();
    if (utf8.empty()) return {};
    const int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), nullptr, 0);
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), wide.data(), wlen);
    return wide;
}

// Convert AUI IDrawable to a pre-multiplied BGRA HBITMAP at 16×16 physical px —
// the classic Win32 menu icon size. DWM composites this correctly on Vista+.
HBITMAP drawableToHBITMAP(const _<IDrawable>& drawable) {
    if (!drawable) return nullptr;
    const glm::ivec2 size { 16, 16 };
    AImage rasterized = drawable->rasterize(size);
    const auto src = rasterized.buffer();
    if (src.empty()) return nullptr;

    const int w = rasterized.width();
    const int h = rasterized.height();
    const int bytesPerRow = w * 4;
    if (int(src.size()) < bytesPerRow * h) return nullptr;

    BITMAPINFO bmi {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;   // negative = top-down rows (AUI stores top-down)
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pixels = nullptr;
    HBITMAP hbmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!hbmp || !pixels) return nullptr;

    // AUI is RGBA; Win32 DIBs are BGRA — swizzle & pre-multiply alpha in one pass.
    const std::uint8_t* srcBytes = reinterpret_cast<const std::uint8_t*>(src.data());
    std::uint8_t* dstBytes = static_cast<std::uint8_t*>(pixels);
    for (int i = 0; i < w * h; ++i) {
        const std::uint8_t r = srcBytes[i * 4 + 0];
        const std::uint8_t g = srcBytes[i * 4 + 1];
        const std::uint8_t b = srcBytes[i * 4 + 2];
        const std::uint8_t a = srcBytes[i * 4 + 3];
        dstBytes[i * 4 + 0] = std::uint8_t((int(b) * a + 127) / 255);
        dstBytes[i * 4 + 1] = std::uint8_t((int(g) * a + 127) / 255);
        dstBytes[i * 4 + 2] = std::uint8_t((int(r) * a + 127) / 255);
        dstBytes[i * 4 + 3] = a;
    }
    return hbmp;
}

HMENU buildMenu(const AVector<AMenuItem>& items, MenuResources& res);

void appendItem(HMENU parent, const AMenuItem& item, MenuResources& res) {
    switch (item.type) {
        case AMenu::SEPARATOR: {
            AppendMenuW(parent, MF_SEPARATOR, 0, nullptr);
            break;
        }
        case AMenu::SUBLIST: {
            HMENU sub = buildMenu(item.subItems, res);
            UINT flags = MF_POPUP | MF_STRING;
            if (!item.enabled) flags |= MF_GRAYED;
            AppendMenuW(parent, flags, reinterpret_cast<UINT_PTR>(sub), toWide(item.name).c_str());
            break;
        }
        case AMenu::SINGLE: {
            const UINT id = res.nextId++;
            res.callbacks.emplace(id, item.onAction);

            // Win32 auto-right-aligns anything after a tab character.
            std::wstring label = toWide(item.name);
            if (!item.shortcut.empty()) {
                const AString shortcutText = static_cast<AString>(item.shortcut);
                if (!shortcutText.empty()) {
                    label.push_back(L'\t');
                    label += toWide(shortcutText);
                }
            }

            UINT flags = MF_STRING;
            if (!item.enabled) flags |= MF_GRAYED;
            AppendMenuW(parent, flags, id, label.c_str());

            if (HBITMAP hbmp = drawableToHBITMAP(item.icon)) {
                res.bitmaps.push_back(hbmp);
                MENUITEMINFOW mii {};
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_BITMAP;
                mii.hbmpItem = hbmp;
                SetMenuItemInfoW(parent, id, FALSE, &mii);
            }
            break;
        }
    }
}

HMENU buildMenu(const AVector<AMenuItem>& items, MenuResources& res) {
    HMENU menu = CreatePopupMenu();
    res.menus.push_back(menu);
    for (const auto& item : items) {
        appendItem(menu, item, res);
    }
    return menu;
}

}   // namespace

Win32MenuProvider::Win32MenuProvider() = default;

Win32MenuProvider::~Win32MenuProvider() {
    closeMenu();
}

void Win32MenuProvider::createMenu(const AVector<AMenuItem>& vector) {
    closeMenu();

    MenuResources res;
    HMENU root = buildMenu(vector, res);
    if (!root) return;

    POINT pt {};
    GetCursorPos(&pt);

    HWND hwnd = nullptr;
    if (auto* base = AWindow::current()) {
        if (auto* win = dynamic_cast<AWindow*>(base)) {
            hwnd = win->getNativeHandle();
        }
    }

    mOpen = true;
    const BOOL cmd = TrackPopupMenuEx(
        root,
        TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN,
        pt.x, pt.y,
        hwnd,
        nullptr);
    mOpen = false;

    if (cmd != 0) {
        const auto it = res.callbacks.find(UINT(cmd));
        if (it != res.callbacks.end() && it->second) {
            it->second();
        }
    }
    // res destructor cleans up HMENU + HBITMAPs.
}

void Win32MenuProvider::closeMenu() {
    if (mOpen) {
        EndMenu();   // unblock the modal TrackPopupMenuEx; createMenu will finish cleanup.
    }
}

bool Win32MenuProvider::isOpen() {
    return mOpen;
}
