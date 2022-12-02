// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <initializer_list>
#include "Rule.h"

class API_AUI_VIEWS AStylesheet {
private:
    AVector<Rule> mRules;
    bool mIgnoreRules = false;

public:
    AStylesheet();
    AStylesheet(std::initializer_list<Rule> rules) {
        addRules(rules);
    }

    void addRules(std::initializer_list<Rule> rules) {
        if (mIgnoreRules) {
            return;
        }
        for (auto& constRule : rules) {
            auto& rule = const_cast<Rule&>(constRule);
            mRules << std::move(rule);
        }
    }

    void addRule(const Rule& r) {
        if (mIgnoreRules) {
            return;
        }
        mRules << r;
    }


    void addRule(Rule&& r) {
        if (mIgnoreRules) {
            return;
        }
        mRules << std::forward<Rule>(r);
    }

    void setIgnoreRules(bool ignoreRules) {
        mIgnoreRules = ignoreRules;
    }


    [[nodiscard]] const AVector<Rule>& getRules() const {
        return mRules;
    }

    static AColor getOsThemeColor();

    static AStylesheet& global();

    void setRules(AVector<Rule> rules) {
        mRules = std::move(rules);
    }

private:
    static _<AStylesheet>& globalStorage();
};
