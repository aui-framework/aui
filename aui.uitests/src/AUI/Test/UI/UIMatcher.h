/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include "IMatcher.h"
#include <gtest/gtest.h>
#include <AUI/Test/UI/Assertion/Empty.h>
#include <AUI/View/AViewContainer.h>
#include "UITestUtil.h"

/**
 * @brief A lazy selector of views in all opened windows, with actions and checks to run on them.
 * @ingroup uitests
 * @details
 * Obtain UIMatcher from the `By` functions (`By::text`, `By::name`, `By::type`, `By::exactType`, `By::value`).
 * The matcher does not remember views: every `perform`, `check`, `one`, `toSet` and `toVector` call walks all windows
 * of the window manager and finds the views that match at that moment. Therefore, the same matcher can be reused after
 * UI has changed.
 *
 * Only views with `Visibility::VISIBLE` are matched (and views inside invisible containers are not reachable), unless
 * `includeInvisibleViews()` is called or an assertion that ignores visibility is used (`visible()`, `notVisible()`,
 * `gone()`, `isParentOf`).
 *
 * ```cpp
 * By::text("Say hello").perform(click());
 * By::name("#hello").check(visible()).check(text("Hello!"));
 * ```
 *
 * Matchers are combined with `operator|` (either) and `operator&` (both):
 *
 * ```cpp
 * (By::text("OK") | By::text("Cancel")).check(sameWidth());
 * ```
 *
 * `perform` and `check` return the matcher itself, so they can be chained.
 *
 * !!! note
 *
 *     `check` fails the test when nothing is matched, and `perform` on an empty selection does nothing, only prints a
 *     message. Do not assume that a `perform` found its target.
 *
 * See [uitests-selecting-views], [uitests-actions] and [uitests-assertions].
 */
class API_AUI_UITESTS UIMatcher {
private:
    _<IMatcher> mMatcher;
    bool mIncludeInvisibleViews = false;


    template<typename Container>
    void processContainer(Container& destination, const _<AViewContainerBase>& container) const;

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
    AVector<_<AView>> toVector() const;

    /**
     * @brief Returns the first matched view, or nullptr if nothing is matched.
     * @details
     * Use it to get the actual view for direct inspection:
     *
     * ```cpp
     * auto field = _cast<ATextField>(By::name("#username").one());
     * ```
     *
     * The order of views is unspecified; when several views match, make the selector more precise.
     */
    _<AView> one() const {
        auto set = toSet();
        if (set.empty()) {
            SCOPED_TRACE("no views selected");
            return nullptr;
        }
        return *set.begin();
    }

    /**
     * @brief Makes this matcher consider views that are not `Visibility::VISIBLE` as well.
     */
    UIMatcher& includeInvisibleViews() {
        mIncludeInvisibleViews = true;
        return *this;
    }

    /**
     * @brief Runs an action (`click()`, `type("...")`, `scroll(...)` etc...) on every matched view.
     * @details
     * The UI is processed (`uitest::frame()`) before the first and after every action.
     * @return this matcher, to chain further calls.
     */
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
     * ```cpp
     * _new<ALabel>("Login"),
     * _new<ATextField>(),
     * _new<ALabel>("Password"),
     * _new<ATextField>(),
     * ...
     * By::type<ATextField>().findNearestTo(By::text("Login")) // <- matches the first ATextField
     * By::type<ATextField>().findNearestTo(By::text("Password")) // <- matches the second ATextField
     * ```
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


    /**
     * @brief Checks that every matched view satisfies the assertion.
     * @param assertion `visible()`, `text("...")`, `width(...)` etc... or any callable `bool(const _<AView>&)`.
     * @param msg message to print when the check fails.
     * @details
     * Failing is non-fatal (`EXPECT_*` semantics): the test continues. An empty selection fails the check.
     * @return this matcher, to chain further calls.
     */
    template<class Assertion>
    UIMatcher& check(Assertion&& assertion, const char* msg = "no msg") {
        // an explicit includeInvisibleViews() is respected; the assertion may only widen the selection for this check.
        const bool includeInvisibleViewsBefore = mIncludeInvisibleViews;
        mIncludeInvisibleViews = includeInvisibleViewsBefore || ignores_visibility<Assertion>::value;
        auto set = toSet();
        EXPECT_FALSE(set.empty()) << msg << ": empty set\n" << AStacktrace::capture(1);

        performHintChecks<Assertion>(msg, set);
        for (auto& s : set) {
            EXPECT_TRUE(assertion(s)) << msg <<  "\n" << AStacktrace::capture(1);
        }
        mIncludeInvisibleViews = includeInvisibleViewsBefore;
        return *this;
    }

    /**
     * @brief Matches containers that have a direct child matched by this matcher.
     */
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

    /**
     * @brief Matches the views whose parent is matched by this matcher.
     * @details
     * Combined with `parent()`, selects siblings: `By::name("a").parent().allChildren()`.
     */
    [[nodiscard]]
    UIMatcher allChildren() const {

        struct ChildMatcher: public IMatcher {
        private:
            _<IMatcher> childMatcher;
        public:
            ChildMatcher(const _<IMatcher>& childMatcher) : childMatcher(childMatcher) {}

            bool matches(const _<AView>& view) override {
                return childMatcher->matches(aui::ptr::fake_shared(view->getParent()));
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


