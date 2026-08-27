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
#include <initguid.h>
#include <KnownFolders.h>
#include <shlobj.h>
#include <AUI/Platform/AFontManager.h>
#include "AUI/Font/FreeType.h"
#include <AUI/IO/APath.h>
#include <AUI/Logging/ALogger.h>

AFontManager::AFontManager()
    : mFreeType(_new<FreeType>())
{
    auto fontPath = [this] {
        for (auto name : {"segoeui", "tahoma"}) {
            APath path = getPathToFont(name);
            if (path.isRegularFileExists()) {
                return path;
            }
        }
        throw AException("No segoeui or tahoma .ttf font is installed into Windows/Fonts folder.");
    }();

    mDefaultFont = loadFont(AUrl::file(fontPath));

    // Pre-warm fallback face to avoid UI-thread stall on first missing-glyph render.
    initFallback();
}

AVector<AFontManager::FallbackCandidate> AFontManager::fallbackCandidates() {
    // Windows font directories are version-specific (see Windows 7 msyh.ttc
    // as the canonical example); the directory itself, however, can be found
    // through the API. Since Windows 8.1, C:\Windows\Fonts contains all
    // system fonts, including the CJK fallback fonts.
    // Order of preference: most-comprehensive first (Microsoft YaHei covers
    // CJK + Kana, Malgun Gothic covers CJK + Hangul, then region-specific
    // fallbacks).
    const AString fontsDir = [] {
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &path))) {
            AString result(reinterpret_cast<const char16_t*>(path));
            result += "\\";
            CoTaskMemFree(path);
            return result;
        }
        ALogger::warn("Font") << "SHGetKnownFolderPath() failed, falling back to GetWindowsDirectoryW()";

        wchar_t buf[MAX_PATH];
        UINT len = GetWindowsDirectoryW(buf, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            return AString(reinterpret_cast<const char16_t*>(buf), len) + "\\Fonts\\";
        }
        ALogger::warn("Font") << "GetWindowsDirectoryW() failed, falling back to C:\\Windows\\Fonts\\";

        return AString("C:\\Windows\\Fonts\\");
    }();
    return {
        { fontsDir + "msyh.ttc" },       // Microsoft YaHei (Simplified Chinese, includes CJK + Kana)
        { fontsDir + "malgun.ttf" },     // Malgun Gothic (Korean, includes Hangul + CJK)
        { fontsDir + "simsun.ttc" },     // SimSun (Simplified Chinese)
        { fontsDir + "msgothic.ttc" },   // MS Gothic (Japanese)
        { fontsDir + "yugothr.ttc" },    // Yu Gothic (Japanese)
        { fontsDir + "meiryo.ttc" },     // Meiryo (Japanese)
    };
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

            std::wstring valueName;
            valueName.resize(maxValueNameSize);
            std::wstring valueData;
            valueData.resize(maxValueDataSize);

            for (DWORD index = 0; RegEnumValue(fontsKey, index, valueName.data(), &valueNameSize, 0, &valueType,
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
                AString u8valueName(reinterpret_cast<const char16_t*>(valueName.data()), valueName.size());
                if (u8valueName.startsWith(font + " (")) {
                    APath path = u8valueName;
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
