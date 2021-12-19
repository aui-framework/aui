#pragma once


#include "IMatcher.h"
#include <boost/test/unit_test.hpp>
#include <AUI/Test/UI/Assertion/Empty.h>

class API_AUI_UITESTS Matcher {
private:
    _<IMatcher> mMatcher;
    ASet<_<AView>> toSet() const;

    void processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const;


    template<class Assertion>
    void performHintChecks(const char* msg, const ASet<_<AView>>& set) const {
        UITest::frame();
        if (msg == nullptr) BOOST_WARN_MESSAGE(false, "Assertion message is empty");
        if constexpr(!std::is_same_v<Assertion, empty>) {
            if (set.empty()) BOOST_WARN_MESSAGE(false, "Matcher is empty so check is not performed");
        }
    }

public:
    Matcher(const _<IMatcher>& matcher) : mMatcher(matcher) {}

    template<class Action>
    void perform(Action&& action) const {
        auto set = toSet();
        if (set.empty()) BOOST_WARN_MESSAGE(false, "Matcher is empty so action is not performed");

        UITest::frame();
        for (auto& v : set) {
            action(v);
            UITest::frame();
        }
    }


    template<class Assertion>
    Matcher& require(Assertion&& assertion, const char* msg = nullptr) {
        auto set = toSet();
        performHintChecks<Assertion>(msg, set);
        for (auto& s : set) {
            BOOST_REQUIRE_MESSAGE(assertion(s), msg);
        }
        return *this;
    }
    template<class Assertion>
    Matcher& warn(Assertion&& assertion, const char* msg = nullptr) {
        auto set = toSet();
        performHintChecks<Assertion>(msg, set);
        for (auto& s : set) {
            BOOST_WARN_MESSAGE(assertion(s), msg);
        }
        return *this;
    }
    template<class Assertion>
    Matcher& check(Assertion&& assertion, const char* msg = nullptr) {
        auto set = toSet();
        performHintChecks<Assertion>(msg, set);
        for (auto& s : set) {
            BOOST_CHECK_MESSAGE(assertion(s), msg);
        }
        return *this;
    }
};


