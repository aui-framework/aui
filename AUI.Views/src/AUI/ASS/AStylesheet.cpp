//
// Created by alex2 on 29.12.2020.
//

#include "AStylesheet.h"
#include "SolidBackground.h"

AStylesheet::AStylesheet() {
    addRules({
        Rule {
            ass::SolidBackground {
                .color = 0xffff0000_argb
            }
        }
    });
}

void AStylesheet::addRules(std::initializer_list<Rule> rules) {
    for (auto&& r : rules) {
        mRules.push_back(r);
    }
}

AStylesheet& AStylesheet::inst() {
    static AStylesheet s;
    return s;
}
