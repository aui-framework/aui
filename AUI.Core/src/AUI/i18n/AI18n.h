//
// Created by alex2 on 07.11.2020.
//

#pragma once

#include <AUI/Util/Singleton.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AMap.h>
#include <AUI/IO/IInputStream.h>

class API_AUI_CORE AI18n: public Singleton<AI18n> {
private:
    AMap<AString, AString> mLangData;

public:
    void loadFromLang(const AString& languageName);
    void loadFromStream(const _<IInputStream>& iis);
};


inline AString operator"" _i18n(const char* input, size_t s)
{
    return input;
}