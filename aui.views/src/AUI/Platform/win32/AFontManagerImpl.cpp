// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

//
// Created by alex2772 on 7/2/21.
//


#include <Windows.h>
#include <AUI/Platform/AFontManager.h>
#include "AUI/Render/FreeType.h"
#include <AUI/IO/APath.h>

AFontManager::AFontManager() :
        mFreeType(_new<FreeType>()),
        mDefaultFont(loadFont(AUrl::file(getPathToFont("segoeui"))))
{
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

            wchar_t* valueName = new wchar_t[maxValueNameSize];
            wchar_t* valueData = new wchar_t[maxValueDataSize];

            for (DWORD index = 0; RegEnumValue(fontsKey, index, valueName, &valueNameSize, 0, &valueType,
                                               reinterpret_cast<LPBYTE>(valueData), &valueDataSize) !=
                                  ERROR_NO_MORE_ITEMS; ++index) {
                valueDataSize = maxValueDataSize;
                valueNameSize = maxValueNameSize;

                if (valueType != REG_SZ) {
                    continue;
                }

                // Found a match
                if (AString(valueName).startsWith(font + " (")) {
                    APath path{valueData, valueDataSize};
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
