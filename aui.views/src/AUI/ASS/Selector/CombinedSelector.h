/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AAssSelector.h"

namespace ass {
    template <typename L, typename R>
    struct CombinedSelector : public IAssSubSelector {
    private:
        L l;
        R r;

    public:
        CombinedSelector(L l, R r) : l(std::move(l)), r(std::move(r)) {}

        bool isPossiblyApplicable(AView* view) override {
            return l.isPossiblyApplicable(view) && r.isPossiblyApplicable(view);
        }

        bool isStateApplicable(AView* view) override {
            return l.isStateApplicable(view) && r.isStateApplicable(view);
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            l.setupConnections(view, helper);
            r.setupConnections(view, helper);
        }
    };

    template <typename L, typename R, std::enable_if_t<std::is_base_of_v<IAssSubSelector, L> && std::is_base_of_v<IAssSubSelector, R>, bool> = true>
    CombinedSelector<L, R> operator+(L l, R r) {
        return CombinedSelector<L, R>(std::move(l), std::move(r));
    }
}