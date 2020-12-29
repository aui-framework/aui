//
// Created by alex2 on 07.11.2020.
//

#pragma once

#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/IO/IInputStream.h>
#include "ALanguageCode.h"

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