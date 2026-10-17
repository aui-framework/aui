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
#include <AUI/Platform/win32/Win32Util.h>

#include <windows.h>

#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>

namespace {

// Ids start at 1 — ID 0 from TrackPopupMenuEx means "cancelled".
constexpr UINT kFirstMenuId = 1;

// Owned resources for one popup lifetime: id→callback map + HBITMAPs for icons. Freed after the modal call returns.
struct MenuResources {
    std::unordered_map<UINT, std::function<void()>> callbacks;
    std::vector<aui::win32::Bitmap> bitmaps;
    UINT nextId = kFirstMenuId;
};

std::wstring toWide(const AString& s) {
    const std::string utf8 = s.toStdString();
    if (utf8.empty()) return {};
    const int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), nullptr, 0);
    std::wstring wide(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), int(utf8.size()), wide.data(), wlen);
    return wide;
}

aui::win32::Menu buildMenu(const AVector<AMenuItem>& items, MenuResources& res);

void appendItem(HMENU parent, const AMenuItem& item, MenuResources& res) {
    switch (item.type) {
        case AMenu::SEPARATOR: {
            AppendMenuW(parent, MF_SEPARATOR, 0, nullptr);
            break;
        }
        case AMenu::SUBLIST: {
            aui::win32::Menu sub = buildMenu(item.subItems, res);
            UINT flags = MF_POPUP | MF_STRING;
            if (!item.enabled) flags |= MF_GRAYED;
            if (AppendMenuW(parent, flags, reinterpret_cast<UINT_PTR>(sub.get()), toWide(item.name).c_str())) {
                sub.release();   // the parent menu owns it now
            }
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

            // 16×16 physical px is the classic Win32 menu icon size.
            if (item.icon) {
                const AImage icon = item.icon->rasterize({ 16, 16 });
                if (icon.buffer().empty()) {
                    break;
                }
                auto& hbmp = res.bitmaps.emplace_back(
                    aui::win32::imageToBitmap(icon, aui::win32::BitmapAlpha::PREMULTIPLIED));
                MENUITEMINFOW mii {};
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_BITMAP;
                mii.hbmpItem = hbmp.get();
                SetMenuItemInfoW(parent, id, FALSE, &mii);
            }
            break;
        }
    }
}

aui::win32::Menu buildMenu(const AVector<AMenuItem>& items, MenuResources& res) {
    aui::win32::Menu menu(CreatePopupMenu());
    for (const auto& item : items) {
        appendItem(menu.get(), item, res);
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
    const aui::win32::Menu root = buildMenu(vector, res);
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
        root.get(),
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
}

void Win32MenuProvider::closeMenu() {
    if (mOpen) {
        EndMenu();   // unblock the modal TrackPopupMenuEx; createMenu will finish cleanup.
    }
}

bool Win32MenuProvider::isOpen() {
    return mOpen;
}
