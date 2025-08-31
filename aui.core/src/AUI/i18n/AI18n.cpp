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
// Created by alex2 on 07.11.2020.
//

#include "AI18n.h"

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Traits/strings.h>
#include <AUI/Url/AUrl.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Platform/ErrorToException.h>

AI18n::AI18n() {
    // TODO hardcoded
    mLangData["aui.cut"] = "Cut";
    mLangData["aui.copy"] = "Copy";
    mLangData["aui.paste"] = "Paste";
    mLangData["aui.select_all"] = "Select all";

    try {
        loadFromLang(userLanguage());
    } catch (...) {
        try {
            loadFromLang("en-US");
            ALogger::warn("AI18n") << "User native language ({}) is not supported, using en-US"_format(userLanguage().toString());
        } catch (...) {
            if (userLanguage().toString() == "en-US") {
                ALogger::err("AI18n") << "US English (en-US) language file has not found! Please add at least en-US.lang file "
                             "if you want to use AUI _i18n feature.";
            } else {
                ALogger::err("AI18n") << "Neither user native ({}) nor US English (en-US) language file have not found! Please"
                             "add at least en-US.lang file if you want to use AUI _i18n feature."_as
                             .format(userLanguage().toString());
            }
        }
    }
}

void AI18n::loadFromStreamInto(const _<IInputStream>& iis, AMap<AString, AString>& langData) {
    ATokenizer t(iis);
    bool running = true;
    try {
        while (running) {
            if (t.readChar() == '#') {
                // comment
                t.skipUntilUnescaped('\n');
            } else {
                // string
                t.reverseByte();

                auto key = AString(t.readStringUntilUnescaped('='));
                std::string value;
                try {
                    t.readStringUntilUnescaped(value, '\n');
                } catch (...) {
                    running = false;
                }

                langData[key] = AString(value).processEscapes();
            }
        }
    } catch (...) {

    }
}

AI18n& AI18n::inst() {
    static AI18n a;
    return a;
}

void AI18n::loadFromLang(const ALanguageCode& languageCode) {
    loadFromStream(AUrl(":lang/{}.lang"_format(languageCode.toString())).open());
}

void AI18n::loadFromStream(const _<IInputStream>& iis) {
    loadFromStreamInto(iis, mLangData);
}

#if AUI_PLATFORM_WIN

#include <windows.h>

ALanguageCode AI18n::userLanguage() {
    AByteBuffer resultU16(LOCALE_NAME_MAX_LENGTH * sizeof(wchar_t));
    int len = GetUserDefaultLocaleName(reinterpret_cast<wchar_t*>(resultU16.data()), LOCALE_NAME_MAX_LENGTH);
    if (len == 0) {
        aui::impl::lastErrorToException("could not get user language");
    }
    return AString(reinterpret_cast<const char16_t*>(resultU16.data()), len);
}

#else

#include <langinfo.h>

ALanguageCode AI18n::userLanguage() {

    char* s;
    s = getenv("LANG");
    if (s) {
        try {
            AString code(s, s + 5);
            code[2] = '-';
            return code;
        } catch (...) {

        }
    }
    return "en-US";
}

#endif