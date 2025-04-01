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

//
// Created by alex2 on 31.12.2020.
//

#pragma once

#include <AUI/Common/AVector.h>
#include <AUI/Util/kAUI.h>
#include <type_traits>
#include <utility>

class AView;
class AAssHelper;

namespace ass {

    class API_AUI_VIEWS IAssSubSelector {
    public:
        virtual bool isPossiblyApplicable(AView* view) = 0;
        virtual bool isStateApplicable(AView* view);
        virtual void setupConnections(AView* view, const _<AAssHelper>& helper);
        virtual ~IAssSubSelector() = default;
    };

    class API_AUI_VIEWS AAssSelector final: public IAssSubSelector {
    private:
        AVector<_<IAssSubSelector>> mSubSelectors;

        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _new<SubSelector>(std::forward<SubSelector>(subSelector));
        }

        template<typename SubSelector, std::enable_if_t<std::is_pointer_v<SubSelector>, bool> = true>
        void processSubSelector(SubSelector&& subSelector) {
            mSubSelectors << _<IAssSubSelector>(subSelector);
        }

        template<typename SubSelector, typename...SubSelectors>
        void processSubSelectors(SubSelector&& subSelector, SubSelectors&&... subSelectors) {
            processSubSelector(std::forward<SubSelector>(subSelector));
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }

        void processSubSelectors() {}

    public:
        template<typename...SubSelectors>
        AAssSelector(SubSelectors&&... subSelectors) {
            processSubSelectors(std::forward<SubSelectors>(subSelectors)...);
        }
        AAssSelector(AAssSelector&& move) noexcept: mSubSelectors(std::move(move.mSubSelectors)) {

        }
        explicit AAssSelector(std::nullptr_t) {}
        AAssSelector(const AAssSelector&) = default;

        ~AAssSelector() {
            mSubSelectors.clear();
        }

        AAssSelector& operator=(const AAssSelector& c) = default;

        bool isPossiblyApplicable(AView* view) const {
            for (const auto& s : mSubSelectors) {
                if (s->isPossiblyApplicable(view)) {
                    return true;
                }
            }
            return false;
        }

        bool isPossiblyApplicable(AView* view) override {
            return constMe()->isPossiblyApplicable(view);
        }

        bool isStateApplicable(AView* view) const {
            for (const auto& s : mSubSelectors) {
                if (s->isStateApplicable(view)) {
                    return true;
                }
            }
            return false;
        }
        bool isStateApplicable(AView* view) override {
            return constMe()->isStateApplicable(view);
        }
        void setupConnections(AView* view, const _<AAssHelper>& helper) const;
        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            constMe()->setupConnections(view, helper);
        }
        template<typename SubSelector, std::enable_if_t<!std::is_pointer_v<SubSelector>, bool> = true>
        void addSubSelector(SubSelector&& subSelector) {
            processSubSelector(std::forward<SubSelector>(subSelector));
        }

        [[nodiscard]]
        const AVector<_<IAssSubSelector>>& getSubSelectors() const {
            return mSubSelectors;
        }

        static AAssSelector makeCopy(const AAssSelector& from) {
            AAssSelector result;
            result.mSubSelectors = from.mSubSelectors;
            return result;
        }

    private:
        const AAssSelector* constMe() {
            // NOLINTNEXTLINE(*-const-cast)
            return const_cast<const AAssSelector*>(this);
        }
    };

    /**
     * @brief Runtime-type selector.
     * @ingroup ass_selectors
     * @details
     * This type type-erases the selectors and allows to encapsulate.
     */
    using sel = AAssSelector;
}