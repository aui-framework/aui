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

//
// Created by alex2772 on 7/2/21.
//


#include <Windows.h>
#include <AUI/Platform/AFontManager.h>
#include "AUI/Font/FreeType.h"
#include <AUI/IO/APath.h>

AFontManager::AFontManager()
    : mFreeType(_new<FreeType>())
{
    auto fontPath = [] {
        for (auto name : {"segoeui", "tahoma"}) {
            APath path = getPathToFont(name);
            if (path.isRegularFileExists()) {
                return path;
            }
        }
        throw AException("No segoeui or tahoma .ttf font is installed into Windows/Fonts folder.");
    }();

    mDefaultFont = loadFont(AUrl::file(fontPath));
}


AString AFontManager::getPathToFont(const AString& font) {

    try {
        if (APath(font.toStdString()).isRegularFileExists())
            return font;
    } catch(...) {}
    try {
        HKEY fontsKey;
        for (auto dir : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER}) {
            if (RegOpenKeyEx(dir, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ,
                             &fontsKey))
                throw std::exception{};
            if (fontsKey == nullptr)
                throw std::exception{};
            DWORD maxValueNameSize = 0, maxValueDataSize = 0;
            DWORD valueNameSize = 0, valueDataSize = 0, valueType = 0;

            if (RegQueryInfoKey(fontsKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0)) {
                throw std::exception{};
            }

            AString valueName;
            valueName.resize(maxValueNameSize);
            AString valueData;
            valueData.resize(maxValueDataSize);

            for (DWORD index = 0; RegEnumValue(fontsKey, index, aui::win32::toWchar(valueName), &valueNameSize, 0, &valueType,
                                               reinterpret_cast<LPBYTE>(valueData.data()), &valueDataSize) !=
                                  ERROR_NO_MORE_ITEMS; ++index) {
                valueDataSize = maxValueDataSize;
                valueNameSize = maxValueNameSize;

                valueData.resize(valueDataSize);
                valueName.resize(valueNameSize);

                if (valueType != REG_SZ) {
                    continue;
                }

                // Found a match
                if (valueName.startsWith(font + " (")) {
                    APath path = valueName;
                    if (path.empty()) continue;
                    if (!path.isRegularFileExists()) continue;

                    RegCloseKey(fontsKey);
                    return path;
                }
            }
        }
        RegCloseKey(fontsKey);
    } catch(...) {}
    return "C:/Windows/Fonts/" + font + ".ttf";
}
