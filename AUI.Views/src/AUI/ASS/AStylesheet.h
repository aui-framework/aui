//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <initializer_list>
#include "Rule.h"

class API_AUI_VIEWS AStylesheet {
private:
    AVector<Rule> mRules;

public:
    AStylesheet();

    inline void addRules(std::initializer_list<Rule> rules) {
        for (auto& constRule : rules) {
            auto& rule = const_cast<Rule&>(constRule);
            mRules << std::move(rule);
        }
    }


    [[nodiscard]] const AVector<Rule>& getRules() const {
        return mRules;
    }

    static AStylesheet& inst();
};


