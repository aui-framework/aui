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
// Created by alex2 on 02.01.2021.
//

#pragma once

#include <AUI/Util/kAUI.h>
#include <AUI/View/AView.h>
#include <AUI/ASS/AAssHelper.h>

#include "hovered.h"
#include "activated.h"
#include "focused.h"
#include "disabled.h"

namespace ass {

    namespace detail {
        struct ClassOf: IAssSubSelector {
        private:
            AStringVector mClasses;
            
        public:
            ClassOf(const AStringVector& classes) : mClasses(classes) {}
            ClassOf(const AString& clazz) : mClasses({clazz}) {}

            bool isPossiblyApplicable(AView* view) override {
                for (auto& v : mClasses) {
                    if (view->getAssNames().contains(v)) {
                        return true;
                    }
                }
                return false;
            }

            bool isStateApplicable(AView* view) override {
                return isPossiblyApplicable(view);
            }

            const AStringVector& getClasses() const {
                return mClasses;
            }
        };
    }

    struct class_of: detail::ClassOf {
    public:
        class_of(const AStringVector& classes) : ClassOf(classes) {}
        class_of(const AString& clazz) : ClassOf(clazz) {}

        using hover = ass::hovered<detail::ClassOf>;
        using active = ass::activated<detail::ClassOf>;
        using focus = ass::focused<detail::ClassOf>;
        using disabled = ass::disabled<detail::ClassOf>;
    };

    /**
     * @brief Selects views that are of the specified classes.
     * @ingroup ass_selectors
     * @details
     * This selector selects views that are of the specified classes. The selector can be accessed explicitly via
     * `ass::class_of`.
     *
     * For example:
     * ```cpp
     * {
     *   c(".btn"),
     *   BackgroundSolid(...),
     * }
     * ```
     * This will select all views that belong to ".btn" ASS class.
     */
    using c = class_of;
}