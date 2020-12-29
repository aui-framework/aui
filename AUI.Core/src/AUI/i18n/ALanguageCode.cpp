//
// Created by alex2 on 07.11.2020.
//

#include <AUI/Common/AException.h>
#include "ALanguageCode.h"

ALanguageCode::ALanguageCode(const AString& str) {
    if (str.length() != 5 || str[2] != '-' || islower(str[2]) || islower(str[3])) {
        throw AException("invalid language code: " + str);
    }
    mGroup[0] = char(str[0]);
    mGroup[1] = char(str[1]);
    mSubGroup[0] = char(str[3]);
    mSubGroup[1] = char(str[4]);
}

