/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 12/5/2021.
//

#include <AUI/View/AText.h>
#include <AUI/Common/IStringable.h>
#include "By.h"

UIMatcher By::text(const AString& text) {
    class TextMatcher: public IMatcher {
    private:
        AString mText;
    public:
        TextMatcher(AString text) : mText(std::move(text)) {}

        ~TextMatcher() override = default;

        bool matches(const AArc<AView>& view) override {
            auto s = IStringable::toString(view);
            return s == mText;
        }
    };
    return { _new<TextMatcher>(text) };
}

UIMatcher By::name(const AString& text) {
    class NameMatcher: public IMatcher {
    private:
        AString mText;
    public:
        NameMatcher(AString text) : mText(std::move(text)) {}

        ~NameMatcher() override = default;

        bool matches(const AArc<AView>& view) override {
            return view->getAssNames().contains(mText);
        }
    };
    return { _new<NameMatcher>(text) };
}

UIMatcher By::value(const AArc<AView>& value) {
    class ValueMatcher: public IMatcher {
    public:
        explicit ValueMatcher(const AArc<AView>& value) : mValue(value) {}

        bool matches(const AArc<AView>& view) override {
            return mValue == view;
        }

    private:
        AArc<AView> mValue;
    };

    return { _new<ValueMatcher>(value) };
}
