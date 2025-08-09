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

#include "AAssSelector.h"

namespace ass {
    template <typename L>
    struct NotSelector : public IAssSubSelector {
    private:
        L l;

    public:
        NotSelector(L l) : l(std::move(l)) {}

        bool isPossiblyApplicable(AView* view) override {
            return !l.isPossiblyApplicable(view);
        }

        bool isStateApplicable(AView* view) override {
            return l.isStateApplicable(view);
        }

        void setupConnections(AView* view, const _<AAssHelper>& helper) override {
            l.setupConnections(view, helper);
        }
    };

    /**
     * @brief Makes a logical "NOT" selector
     * @ingroup ass_selectors
     * @details
     *
     * In this example, we want to select all views that match ".btn" but don't match ".accent".
     * ```cpp
     * {
     *   class_of(".btn-title") && !class_of(".accent"),
     *   BackgroundSolid { 0xe81123_rgb }
     * },
     * ```
     */
    template <aui::derived_from<IAssSubSelector> L>
    NotSelector<L> operator!(L l) {
        return NotSelector<L>(std::move(l));
    }
}