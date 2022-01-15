#pragma once


#include "IMatcher.h"
#include <gtest/gtest.h>
#include <AUI/Test/UI/Assertion/Empty.h>
#include <AUI/View/AViewContainer.h>
#include "UITestUtil.h"

class API_AUI_UITESTS UIMatcher {
private:
    _<IMatcher> mMatcher;
    bool mIncludeInvisibleViews = false;


    void processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const;

    template<typename T, typename = int>
    struct ignores_visibility : std::false_type { };

    template<typename T>
    struct ignores_visibility<T, decltype((T::IGNORE_VISIBILITY::value, 0))> : T::IGNORE_VISIBILITY { };

    template<class Assertion>
    void performHintChecks(const char* msg, ASet<_<AView>>& set) {
        currentImpl() = this;
        if constexpr (ignores_visibility<Assertion>::value) {
            mIncludeInvisibleViews = true;
            set = toSet();
        }
        UITest::frame();
        if (msg == nullptr) std::cout << "Assertion message is empty";
        if constexpr(!std::is_same_v<Assertion, empty>) {
            if (set.empty()) std::cout << "UIMatcher is empty so check is not performed";
        }
    }

    static UIMatcher*& currentImpl();

public:
    UIMatcher(const _<IMatcher>& matcher) : mMatcher(matcher) {}

    static UIMatcher* current() {
        return currentImpl();
    }

    ASet<_<AView>> toSet() const;

    UIMatcher& includeInvisibleViews() {
        mIncludeInvisibleViews = true;
        return *this;
    }

    template<class Action>
    UIMatcher& perform(Action&& action) {
        auto set = toSet();
        if (set.empty()) std::cout << "UIMatcher is empty so action is not performed";

        UITest::frame();
        for (auto& v : set) {
            action(v);
            UITest::frame();
        }
        return *this;
    }


    template<class Assertion>
    UIMatcher& check(Assertion&& assertion, const char* msg = "no msg") {
        auto set = toSet();
        performHintChecks<Assertion>(msg, set);
        for (auto& s : set) {
            EXPECT_TRUE(assertion(s)) << msg;
        }
        return *this;
    }

    [[nodiscard]]
    UIMatcher parent() const {

        struct ParentMatcher: public IMatcher {
        private:
            _<IMatcher> childMatcher;
        public:
            ParentMatcher(const _<IMatcher>& childMatcher) : childMatcher(childMatcher) {}

            bool matches(const _<AView>& view) override {
                if (auto container = _cast<AViewContainer>(view)) {
                    for (const auto& childView : container) {
                        if (childMatcher->matches(childView)) return true;
                    }
                }
                return false;
            }
        };
        return { _new<ParentMatcher>(mMatcher) };
    }

    [[nodiscard]]
    UIMatcher allChildren() const {

        struct ChildMatcher: public IMatcher {
        private:
            _<IMatcher> childMatcher;
        public:
            ChildMatcher(const _<IMatcher>& childMatcher) : childMatcher(childMatcher) {}

            bool matches(const _<AView>& view) override {
                return childMatcher->matches(aui::ptr::fake(view->getParent()));
            }
        };

        return { _new<ChildMatcher>(mMatcher) };
    }

private:

    template<class BinaryOperator>
    struct BinaryOperatorMatcher: public IMatcher {
    private:
        _<IMatcher> lhs;
        _<IMatcher> rhs;
    public:
        BinaryOperatorMatcher(const _<IMatcher>& lhs, const _<IMatcher>& rhs) : lhs(lhs), rhs(rhs) {}

        bool matches(const _<AView>& view) override {
            return BinaryOperator()(lhs->matches(view), rhs->matches(view));
        }
    };

public:

    UIMatcher operator|(const UIMatcher& matcher) const {
        struct compare_or {
            bool operator()(bool lhs, bool rhs) const {
                return lhs || rhs;
            }
        };
        return { _new<BinaryOperatorMatcher<compare_or>>(mMatcher, matcher.mMatcher) };
    }

    UIMatcher operator&(const UIMatcher& matcher) const {
        struct compare_and {
            bool operator()(bool lhs, bool rhs) const {
                return lhs && rhs;
            }
        };
        return { _new<BinaryOperatorMatcher<compare_and>>(mMatcher, matcher.mMatcher) };
    }
};


