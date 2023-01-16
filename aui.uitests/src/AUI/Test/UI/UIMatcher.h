// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
            set = toSet();
        }
        uitest::frame();
        if (msg == nullptr) std::cout << "Assertion message is empty";
        if constexpr(!std::is_same_v<Assertion, empty>) {
            if (set.empty()) std::cout << "UIMatcher is empty so check is not performed";
        }
    }

    static UIMatcher*& currentImpl();

public:
    UIMatcher(const _<IMatcher>& matcher) : mMatcher(matcher) {}

    ~UIMatcher() {
        if (current() == this) {
            currentImpl() = nullptr;
        }
    }

    static UIMatcher* current() {
        return currentImpl();
    }

    ASet<_<AView>> toSet() const;

    _<AView> one() const {
        auto set = toSet();
        if (set.empty()) {
            SCOPED_TRACE("no views selected");
            return nullptr;
        }
        return *set.begin();
    }

    UIMatcher& includeInvisibleViews() {
        mIncludeInvisibleViews = true;
        return *this;
    }

    template<class Action>
    UIMatcher& perform(Action&& action) {
        auto set = toSet();
        if (set.empty()) std::cout << "UIMatcher is empty so action is not performed";

        uitest::frame();
        for (auto& v : set) {
            action(v);
            uitest::frame();
        }
        return *this;
    }

    /**
     * @brief Finds the nearest view to the specified one.
     * @details
     * Finds the nearest view to the bottom right corner of the specified view. The bottom right corner is chosen in
     * order to simulate human eye scanning (which is from top-left to bottom-right).
     *
     * Useful when finding fields by their labels:
     * @code{cpp}
     * _new<ALabel>("Login"),
     * _new<ATextField>(),
     * _new<ALabel>("Password"),
     * _new<ATextField>(),
     * ...
     * By::type<ATextField>().findNearestTo(By::text("Login")) // <- matches the first ATextField
     * By::type<ATextField>().findNearestTo(By::text("Password")) // <- matches the second ATextField
     * @endcode
     * @param matcher UIMatcher of a view to find the nearest to. The UIMatcher is expected to match only one view.
     * @return the nearest view
     */
    UIMatcher findNearestTo(UIMatcher matcher) {
        auto mySet = toSet();
        auto targets = matcher.toSet();

        if (targets.size() != 1) {
            throw AException("expected to match one view, matched {}"_format(mySet.size()));
        }
        if (mySet.empty()) {
            throw AException("findNearestTo requires at least one element to match");
        }

        auto nearestToView = (*targets.begin());
        auto nearestToPoint = glm::vec2(nearestToView->getPositionInWindow() + nearestToView->getSize());
        auto target = std::min_element(mySet.begin(), mySet.end(), [&](const _<AView>& lhs, const _<AView>& rhs) {
            float dst1 = glm::distance2(nearestToPoint, glm::vec2(lhs->getCenterPointInWindow()));
            float dst2 = glm::distance2(nearestToPoint, glm::vec2(rhs->getCenterPointInWindow()));
            return dst1 < dst2;
        });
        EXPECT_TRUE(target != mySet.end());

        class ToOneMatcher: public IMatcher {
        public:
            explicit ToOneMatcher(_<AView> view) : mView(std::move(view)) {}

            bool matches(const _<AView>& view) override {
                return view == mView;
            }
        private:
            _<AView> mView;
        };

        return { _new<ToOneMatcher>(std::move(*target)) };
    }


    template<class Assertion>
    UIMatcher& check(Assertion&& assertion, const char* msg = "no msg") {
        mIncludeInvisibleViews = ignores_visibility<Assertion>::value;
        auto set = toSet();
        EXPECT_FALSE(set.empty()) << msg << ": empty set";
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


