/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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

void ADesktop::setMousePos(const glm::ivec2& pos) {
    SetCursorPos(pos.x, pos.y);
}


AFuture<APath> ADesktop::browseForDir(ABaseWindow* parent, const APath& startingLocation) {
    AUI_NULLSAFE(parent)->blockUserInput();
    return async noexcept {
        APath result;
        Ole::inst();
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                   IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        AUI_ASSERT(SUCCEEDED(hr));


        ARaiiHelper d = [&] {
            AUI_NULLSAFE(parent)->getThread()->enqueue([parent, pFileOpen] {
                    parent->blockUserInput(false);

                    pFileOpen->Release();
                });
        };

        pFileOpen->SetOptions(FOS_PICKFOLDERS);
        {
            IShellItem* psiFolder = nullptr;
            for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                APath current = i;
                current.replaceAll('/', '\\');
                SHCreateItemFromParsingName(aui::win32::toWchar(current), nullptr, IID_IShellItem,
                                            reinterpret_cast<void**>(&psiFolder));
            }
            if (psiFolder) {
                pFileOpen->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }


        HWND nativeParentWindow;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = d->nativeHandle();
        } else {
            nativeParentWindow = nullptr;
        }

        hr = pFileOpen->Show(nativeParentWindow);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // Display the file name to the user.
                if (SUCCEEDED(hr))
                {
                    result = aui::win32::fromWchar(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }

        return result;
    };
}

AFuture<APath> ADesktop::browseForFile(ABaseWindow* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    AUI_NULLSAFE(parent)->blockUserInput();
    return async noexcept {
        APath result;
        Ole::inst();
        IFileOpenDialog *pFileOpen;

        // Create the FileOpenDialog object.
        auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                   IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));


        ARaiiHelper d = [&] {
            AUI_NULLSAFE(parent)->getThread()->enqueue([parent, pFileOpen] {
                    parent->blockUserInput(false);

                    pFileOpen->Release();
                });
        };

        AUI_ASSERT(SUCCEEDED(hr));
        AVector<COMDLG_FILTERSPEC> filter;
        AVector<AString> storage;
        filter.reserve(extensions.size());
        storage.reserve(extensions.size() * 2);
        for (auto& ext : extensions) {
            auto extFilter = "*." + ext.extension;
            storage << extFilter;
            storage << ext.name + " (" + extFilter + ")";
            filter << COMDLG_FILTERSPEC{ aui::win32::toWchar(*(storage.end()-1)), aui::win32::toWchar(*(storage.end()-2)) };
        }


        hr = pFileOpen->SetFileTypes(filter.size(), filter.data());
        AUI_ASSERT(SUCCEEDED(hr));

        {
            IShellItem* psiFolder = nullptr;
            for (APath i = startingLocation; !i.empty() && !psiFolder; i = i.parent()) {
                APath current = i;
                current.replaceAll('/', '\\');
                SHCreateItemFromParsingName(aui::win32::toWchar(current), nullptr, IID_IShellItem,
                                            reinterpret_cast<void**>(&psiFolder));
            }
            if (psiFolder) {
                pFileOpen->SetFolder(psiFolder);
                psiFolder->Release();
            }
        }

        HWND nativeParentWindow;
        if (auto d = dynamic_cast<AWindow*>(parent)) {
            nativeParentWindow = d->nativeHandle();
        } else {
            nativeParentWindow = nullptr;
        }

        hr = pFileOpen->Show(nativeParentWindow);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                // Display the file name to the user.
                if (SUCCEEDED(hr))
                {
                    result = aui::win32::fromWchar(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }


        return result;
    };
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    SHFILEINFO info;
    if (SUCCEEDED(SHGetFileInfo(aui::win32::toWchar(file), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))) {

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