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
// Created by alex2772 on 1/3/21.
//

#pragma once

#include "AAssSelector.h"
#include <AUI/View/AViewContainer.h>

namespace ass {
    template <typename L, typename R>
    struct ParentSubSelector: public IAssSubSelector {
    private:
        L l;
        R r;

    public:
        ParentSubSelector(L l, R r) : l(std::move(l)), r(std::move(r)) {}

        bool isPossiblyApplicable(AView* view) override {
            if (r.isPossiblyApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isPossiblyApplicable(v)) {
                        return true;
                    }
                }
            }
            return false;
        }

        bool isStateApplicable(AView* view) override {
            if (r.isStateApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isStateApplicable(v) && l.isPossiblyApplicable(v)) {
                        return true;
                    }
                }
            }
            return false;
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            if (r.isPossiblyApplicable(view)) {
                for (AView* v = view->getParent(); v; v = v->getParent()) {
                    if (l.isPossiblyApplicable(v)) {
                        l.setupConnections(v, helper);
                        r.setupConnections(view, helper);
                        return;
                    }
                }
            }
            /**
             * you should never reach here because this function is called only in case isPossiblyApplicable returned
             * true
             */
            AUI_ASSERT(0);
        }
    };

    /**
     * @brief Makes indirect parent subselector.
     * @ingroup ass_selectors
     * @details
     *
     * This selector is slower than direct parent subselector. If possible, use the latter.
     *
     * In this example, we want to select all views that have indirect parent matching ".window-title" ASS class and
     * then select the hovered ".close" subselector within them.
     * ```cpp
     * {
     *   class_of(".window-title") >> class_of::hover(".close"),
     *   BackgroundSolid { 0xe81123_rgb }
     * },
     * ```
     */
    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<IAssSubSelector, L> && std::is_base_of_v<IAssSubSelector, R>, bool> = true>
    ParentSubSelector<L, R> operator>>(L l, R r) {
        return ParentSubSelector<L, R>(std::move(l), std::move(r));
    }
}