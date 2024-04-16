// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by alex2 on 07.11.2020.
//

#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/IO/IInputStream.h>
#include "ALanguageCode.h"

/**
 * @brief Provides i18n (internationalization) support.
 * @ingroup core
 */
class API_AUI_CORE AI18n {
    friend class Lang;
private:
    AMap<AString, AString> mLangData;

    static void loadFromStreamInto(const _<IInputStream>& iis, AMap<AString, AString>& langData);
public:
    AI18n();

    static AI18n& inst();

    void loadFromLang(const ALanguageCode& languageCode);
    void loadFromStream(const _<IInputStream>& iis);

    static ALanguageCode userLanguage();

    const AMap<AString, AString>& getLangData() const {
        return mLangData;
    }
};


inline AString operator"" _i18n(const char* input, size_t s)
{
    auto i = AI18n::inst().getLangData().find(AString(input));
    if (i == AI18n::inst().getLangData().end())
        return input;
    return i->second;
}