//
// Created by alex2 on 26.11.2020.
//

#pragma once


#include <AUI/Common/AString.h>

namespace AClipboard {
    void copyToClipboard(const AString& text);
    AString pasteFromClipboard();
}