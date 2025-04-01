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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <initializer_list>
#include "Rule.h"

class API_AUI_VIEWS AStylesheet {
private:
    AVector<ass::Rule> mRules;
    bool mIgnoreRules = false;

public:
    AStylesheet();
    AStylesheet(std::initializer_list<ass::Rule> rules) {
        addRules(rules);
    }

    void addRules(std::initializer_list<ass::Rule> rules) {
        if (mIgnoreRules) {
            return;
        }
        for (auto& constRule : rules) {
            auto& rule = const_cast<ass::Rule&>(constRule);
            mRules << std::move(rule);
        }
    }

    void addRule(const ass::Rule& r) {
        if (mIgnoreRules) {
            return;
        }
        mRules << r;
    }


    void addRule(ass::Rule&& r) {
        if (mIgnoreRules) {
            return;
        }
        mRules << std::forward<ass::Rule>(r);
    }

    void setIgnoreRules(bool ignoreRules) {
        mIgnoreRules = ignoreRules;
    }


    [[nodiscard]] const AVector<ass::Rule>& getRules() const {
        return mRules;
    }

    static AColor getOsThemeColor();

    static AStylesheet& global();

    void setRules(AVector<ass::Rule> rules) {
        mRules = std::move(rules);
    }

private:
    static _<AStylesheet>& globalStorage();
};
