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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ACursor.h>
#include "AUI/Platform/win32/Ole.h"
#include "AUI/Util/AImageDrawable.h"
#include "AUI/Platform/win32/Win32Util.h"
#include <windows.h>
#include <shlobj.h>
#include <AUI/Traits/memory.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Logging/ALogger.h>

glm::ivec2 ADesktop::getMousePosition() {
    POINT p;
    GetCursorPos(&p);
    return {p.x, p.y};
}

void ADesktop::setMousePos(const glm::ivec2& pos) { SetCursorPos(pos.x, pos.y); }

AFuture<APath> ADesktop::browseForDir(AWindowBase* parent, const APath& startingLocation) {
    _<AAbstractThread> ui;
    if (auto* w = dynamic_cast<AWindow*>(parent))
        ui = w->getThread();
    else
        ui = AThread::current();

    AFuture<APath> promise;

    ui->enqueue([promise, parent, startingLocation]() mutable {
        Ole::inst();
        APath result;

        IFileOpenDialog* dlg = nullptr;
        // Create the FileOpenDialog object.
        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, reinterpret_cast<void**>(&dlg));
        if (SUCCEEDED(hr)) {
            ARaiiHelper release = [&] { dlg->Release(); };
            dlg->SetOptions(FOS_PICKFOLDERS);

            if (!startingLocation.empty()) {
                IShellItem* psiFolder = nullptr;
                for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                    APath current = i;
                    current.replaceAll('/', '\\');
                    auto wcurrent = current.toWideString();
                    SHCreateItemFromParsingName(
                        wcurrent.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psiFolder));
                }
                if (psiFolder) {
                    dlg->SetFolder(psiFolder);
                    psiFolder->Release();
                }
            }

            HWND owner = nullptr;
            if (auto* win = dynamic_cast<AWindow*>(parent))
                owner = win->nativeHandle();

            hr = dlg->Show(owner);
            // Get the file name from the dialog box.
            if (SUCCEEDED(hr)) {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dlg->GetResult(&item))) {
                    PWSTR psz = nullptr;
                    // Display the file name to the user.
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &psz))) {
                        result = AString(reinterpret_cast<const char16_t*>(psz));
                        CoTaskMemFree(psz);
                    }
                    item->Release();
                }
            }
        }

        promise.supplyValue(result);   // resolves with {} on cancel or error
    });

    return promise;
}

AFuture<APath> ADesktop::browseForFile(AWindowBase* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    _<AAbstractThread> ui;
    if (auto* w = dynamic_cast<AWindow*>(parent))
        ui = w->getThread();
    else
        ui = AThread::current();

    AFuture<APath> promise;

    ui->enqueue([promise, parent, startingLocation, extensions]() mutable {
        Ole::inst();
        APath result;

        IFileOpenDialog* dlg = nullptr;
        // Create the FileOpenDialog object.
        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, reinterpret_cast<void**>(&dlg));
        if (SUCCEEDED(hr)) {
            ARaiiHelper release = [&] { dlg->Release(); };

            // Build filters
            AVector<COMDLG_FILTERSPEC> filter;
            AVector<AString> storage;
            filter.reserve(extensions.size());
            storage.reserve(extensions.size() * 2);

            for (const auto& ext : extensions) {
                auto extMask = "*." + ext.extension;
                storage << extMask;
                storage << ext.name + " (" + extMask + ")";
                auto w1 = (*(storage.end() - 1)).toWideString();
                auto w2 = (*(storage.end() - 2)).toWideString();
                filter << COMDLG_FILTERSPEC {
                    w1.c_str(), w2.c_str()
                };
            }

            if (!filter.empty()) {
                dlg->SetFileTypes(static_cast<UINT>(filter.size()), filter.data());
                dlg->SetFileTypeIndex(1);
            }

            // Set starting folder
            if (!startingLocation.empty()) {
                IShellItem* psiFolder = nullptr;
                for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                    APath current = i;
                    current.replaceAll('/', '\\');
                    auto wcurrent = current.toWideString();
                    SHCreateItemFromParsingName(
                        wcurrent.c_str(), nullptr, IID_IShellItem, reinterpret_cast<void**>(&psiFolder));
                }
                if (psiFolder) {
                    dlg->SetFolder(psiFolder);
                    psiFolder->Release();
                }
            }

            HWND owner = nullptr;
            if (auto* win = dynamic_cast<AWindow*>(parent))
                owner = win->nativeHandle();

            hr = dlg->Show(owner);
            // Get the file name from the dialog box.
            if (SUCCEEDED(hr)) {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dlg->GetResult(&item))) {
                    PWSTR psz = nullptr;
                    // Display the file name to the user.
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &psz))) {
                        result = AString(reinterpret_cast<const char16_t*>(psz));
                        CoTaskMemFree(psz);
                    }
                    item->Release();
                }
            }
        }

        promise.supplyValue(result);   // cancels with empty path
    });

    return promise;
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    SHFILEINFO info;
    auto wfile = file.toWideString();
    if (SUCCEEDED(SHGetFileInfo(wfile.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))) {

        ARaiiHelper destroyer = [&]{ DestroyIcon(info.hIcon); };
        return _new<AImageDrawable>(_new<AImage>(aui::win32::iconToImage(info.hIcon)));
    }
    return nullptr;
}

void ADesktop::playSystemSound(ADesktop::SystemSound s)
{
    switch (s)
    {
        case ADesktop::SystemSound::QUESTION:
            PlaySound(L"SystemQuestion", nullptr, SND_ASYNC);
            break;

        case ADesktop::SystemSound::ASTERISK:
            PlaySound(L"SystemAsterisk", nullptr, SND_ASYNC);
            break;

    }
}
