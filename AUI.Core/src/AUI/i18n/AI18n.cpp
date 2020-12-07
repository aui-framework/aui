//
// Created by alex2 on 07.11.2020.
//

#include <AUI/Url/AUrl.h>
#include <AUI/Util/ATokenizer.h>
#include "AI18n.h"
#include <AUI/Traits/strings.h>
#include <AUI/Logging/ALogger.h>


AI18n::AI18n() {
    try {
        loadFromLang(userLanguage());
    } catch (...) {
        try {
            loadFromLang("en-US");
            ALogger::warn("User native language ({}) is not supported, using en-US"_as.format(userLanguage().toString()));
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
    try {
        for (;;) {
            if (t.readChar() == '#') {
                // комментарий
                t.skipUntilUnescaped('n');
            } else {
                // строка
                t.reverseByte();

                auto key = t.readStringUntilUnescaped('=');
                auto value = t.readStringUntilUnescaped('n');

                langData[key] = value;
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
    loadFromStream(AUrl(":lang/{}.lang"_as.format(languageCode.toString())).open());
}

void AI18n::loadFromStream(const _<IInputStream>& iis) {
    loadFromStreamInto(iis, mLangData);
}

#ifdef _WIN32

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