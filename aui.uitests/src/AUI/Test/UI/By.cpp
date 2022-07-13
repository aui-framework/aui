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
