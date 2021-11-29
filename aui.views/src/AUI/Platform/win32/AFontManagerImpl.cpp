/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2772 on 7/2/21.
//


#include <Windows.h>
#include <AUI/Platform/AFontManager.h>
#include "AUI/Render/FreeType.h"
#include <AUI/IO/APath.h>

AFontManagerImpl::AFontManagerImpl() :
        mFreeType(_new<FreeType>()),
        mDefaultFont(loadFont(AUrl::file(getPathToFont("segoeui"))))
{
}


AString AFontManagerImpl::getPathToFont(const AString& font) {

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
                    RegCloseKey(fontsKey);
                    return AString{valueData, valueDataSize};
                }
            }
        }
        RegCloseKey(fontsKey);
    } catch(...) {}
	return "C:/Windows/Fonts/" + font + ".ttf";
}
