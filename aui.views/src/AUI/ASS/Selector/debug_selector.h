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
        struct debug_selector : virtual IAssSubSelector {
        public:
            debug_selector() = default;

            bool isStateApplicable(AView* view) override {
                return false;
            }

            bool isPossiblyApplicable(AView* view) override {
                return false;
            }
        };
    }

    struct debug_selector: detail::debug_selector { // ignore ass_selectors
    public:
        debug_selector() = default;

        using hover = ass::hovered<detail::debug_selector>;
        using active = ass::activated<detail::debug_selector>;
        using focus = ass::focused<detail::debug_selector>;
        using disabled = ass::disabled<detail::debug_selector>;
    };

}