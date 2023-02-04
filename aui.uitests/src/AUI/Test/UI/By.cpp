// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

        bool matches(const _<AView>& view) override {
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

        bool matches(const _<AView>& view) override {
            return view->getAssNames().contains(mText);
        }
    };
    return { _new<NameMatcher>(text) };
}
