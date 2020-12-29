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
    void addRules(std::initializer_list<Rule> rules);

    [[nodiscard]] const AVector<Rule>& getRules() const {
        return mRules;
    }

    static AStylesheet& inst();
};


