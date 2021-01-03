#pragma once

#include <AUI/ASS/Selector/Selector.h>


struct Rule: RuleWithoutSelector {
public:
    template<typename... Declarations>
    Rule(ass::ASelector&& selector, Declarations&&... declarations):
            RuleWithoutSelector(std::forward<Declarations>(declarations)...),
            mSelector(std::forward<ass::ASelector>(selector)) {
    }

    [[nodiscard]] const ass::ASelector& getSelector() const {
        return mSelector;
    }

private:
    ass::ASelector mSelector;
};