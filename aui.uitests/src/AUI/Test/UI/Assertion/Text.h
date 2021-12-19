//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


#include <AUI/Common/IStringable.h>

struct ViewAssertionText {
    AString text;

    explicit ViewAssertionText(const AString& text) : text(text) {}

    bool operator()(const _<AView>& v) {
        return IStringable::toString(v) == text;
    }
};

using text = ViewAssertionText;