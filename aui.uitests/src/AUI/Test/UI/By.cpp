//
// Created by Alex2772 on 12/5/2021.
//

#include <AUI/View/AText.h>
#include <AUI/Common/IStringable.h>
#include "By.h"

Matcher By::text(const AString& text) {
    class TextMatcher: public IMatcher {
    private:
        AString mText;
    public:
        TextMatcher(AString text) : mText(std::move(text)) {}

        ~TextMatcher() override = default;

        bool matches(const _<AView>& view) override {
            return IStringable::toString(view) == mText;
        }
    };
    return { _new<TextMatcher>(text) };
}
