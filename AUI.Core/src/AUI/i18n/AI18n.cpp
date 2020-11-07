//
// Created by alex2 on 07.11.2020.
//

#include <AUI/Url/AUrl.h>
#include <AUI/Util/ATokenizer.h>
#include "AI18n.h"
#include <AUI/Traits/strings.h>


void AI18n::loadFromLang(const AString& languageName) {
    loadFromStream(AUrl(":lang/{}.lang"_as.format(languageName)).open());
}

void AI18n::loadFromStream(const _<IInputStream>& iis) {
    ATokenizer t(iis);
    try {
        for (;;) {
            if (t.readChar() == '#') {
                // комментарий
                t.skipUntilUnescaped('\n');
            } else {
                // строка
                t.reverseByte();

                auto key = t.readStringUntilUnescaped('=');
                auto value = t.readStringUntilUnescaped('\n');

                mLangData[key] = value;
            }
        }
    } catch (...) {

    }
}
