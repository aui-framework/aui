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


inline AString operator""_i18n(const char* input, size_t s)
{
    auto i = AI18n::inst().getLangData().find(AString(input));
    if (i == AI18n::inst().getLangData().end())
        return input;
    return i->second;
}