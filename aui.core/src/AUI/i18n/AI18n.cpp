/*
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
// Created by alex2 on 07.11.2020.
//

#include <AUI/Url/AUrl.h>
#include <AUI/Util/ATokenizer.h>
#include "AI18n.h"
#include <AUI/Traits/strings.h>
#include <AUI/Logging/ALogger.h>


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
            ALogger::warn("User native language ({}) is not supported, using en-US"_format(userLanguage().toString()));
        } catch (...) {
            if (userLanguage().toString() == "en-US") {
                ALogger::err("US English (en-US) language file has not found! Please add at least en-US.lang file "
                             "if you want to use AUI _i18n feature.");
            } else {
                ALogger::err("Neither user native ({}) nor US English (en-US) language file have not found! Please"
                             "add at least en-US.lang file if you want to use AUI _i18n feature."_as
                             .format(userLanguage().toString()));
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

                auto key = t.readStringUntilUnescaped('=');
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
    wchar_t buf[64];
    GetUserDefaultLocaleName(buf, sizeof(buf) / 2);
    return AString(buf);
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