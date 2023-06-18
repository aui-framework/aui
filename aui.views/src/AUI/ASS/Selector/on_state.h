#pragma once

#include <AUI/ASS/Rule.h>

namespace ass::on_state {
    namespace impl {
        struct OnStateSelector: IAssSubSelector {
        public:
            bool isPossiblyApplicable(AView* view) override {
                return true;
            }
        };
    }

    struct Hovered: public Rule {
    public:
        template<typename... Declarations>
        Hovered(Declarations&&... declarations):
            Rule(hovered<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };
}