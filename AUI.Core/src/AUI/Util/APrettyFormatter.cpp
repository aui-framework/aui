/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
