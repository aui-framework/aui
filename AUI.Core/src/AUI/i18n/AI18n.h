//
// Created by alex2 on 07.11.2020.
//

#pragma once

#include <AUI/Util/Singleton.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/IO/IInputStream.h>
#include "ALanguageCode.h"

class API_AUI_CORE AI18n: public Singleton<AI18n> {
    friend class Lang;
private:
    AMap<AString, AString> mLangData;

    static void loadFromStreamInto(const _<IInputStream>& iis, AMap<AString, AString>& langData);
public:

    AI18n();
    void loadFromLang(const ALanguageCode& languageCode);

    void loadFromStream(const _<IInputStream>& iis);

    static ALanguageCode userLanguage();

    const AMap<AString, AString>& getLangData() const {
        return mLangData;
    }
};


inline AString operator"" _i18n(const char* input, size_t s)
{
    return AI18n::instance().getLangData().at(AString(input));
}