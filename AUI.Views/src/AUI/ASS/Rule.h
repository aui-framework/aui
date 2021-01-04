#pragma once

#include <AUI/ASS/Selector/AAssSelector.h>


struct Rule: RuleWithoutSelector {
public:
    template<typename... Declarations>
    Rule(ass::AAssSelector&& selector, Declarations&&... declarations):
            RuleWithoutSelector(std::forward<Declarations>(declarations)...),
            mSelector(std::forward<ass::AAssSelector>(selector)) {
    }

    [[nodiscard]] const ass::AAssSelector& getSelector() const {
        return mSelector;
    }

private:
    ass::AAssSelector mSelector;
};