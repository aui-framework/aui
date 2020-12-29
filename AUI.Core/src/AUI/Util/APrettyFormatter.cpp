//
// Created by alex2 on 28.11.2020.
//

#include "APrettyFormatter.h"
#include <AUI/i18n/AI18n.h>

AString APrettyFormatter::sizeInBytes(uint64_t b) {
    unsigned char postfix_level = 0;
    b *= 10;
    while (b > 20480)
    {
        b = b / 1024;
        postfix_level++;
    }
    auto n = AString::number(b / 10);
    if (b % 10 != 0) {
        n += "." + AString::number(b % 10);
    }
    switch (postfix_level)
    {
        case 0:
            return n + " b"_i18n;
        case 1:
            return n + " Kb"_i18n;
        case 2:
            return n + " Mb"_i18n;
        case 3:
            return n + " Gb"_i18n;
        case 4:
            return n + " Tb"_i18n;
        case 5:
            return n + " Pb"_i18n;
        default:
            return n + " ??"_i18n;
    }
}
