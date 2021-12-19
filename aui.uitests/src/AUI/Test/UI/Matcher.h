#pragma once


#include "IMatcher.h"
#include <boost/test/unit_test.hpp>
#include <AUI/Test/UI/Assertion/Empty.h>

class API_AUI_UITESTS Matcher {
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
        if constexpr (ignores_visibility<Assertion>::value) {
            mIncludeInvisibleViews = true;
            set = toSet();
        }
        UITest::frame();
        if (msg == nullptr) BOOST_WARN_MESSAGE(false, "Assertion message is empty");
        if constexpr(!std::is_same_v<Assertion, empty>) {
            if (set.empty()) BOOST_WARN_MESSAGE(false, "Matcher is empty so check is not performed");
        }
    }

    static Matcher*& currentImpl();

public:
    Matcher(const _<IMatcher>& matcher) : mMatcher(matcher) {}

    static Matcher* current() {
        return currentImpl();
    }

    ASet<_<AView>> toSet() const;

    Matcher& includeInvisibleViews() {
        mIncludeInvisibleViews = true;
        return *this;
    }

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
        currentImpl() = this;
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


