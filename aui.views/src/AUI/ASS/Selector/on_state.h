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

#include <AUI/ASS/Rule.h>
#include <AUI/ASS/PropertyListRecursive.h>

namespace ass::on_state {
    namespace impl {
        struct OnStateSelector: IAssSubSelector {
        public:
            bool isPossiblyApplicable(AView* view) override {
                return true;
            }
        };
    }

    struct Hovered: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Hovered(Declarations&&... declarations):
                ConditionalPropertyList(hovered<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    struct Activated: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Activated(Declarations&&... declarations):
                ConditionalPropertyList(activated<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    struct Focused: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Focused(Declarations&&... declarations):
                ConditionalPropertyList(focused<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    struct Disabled: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Disabled(Declarations&&... declarations):
                ConditionalPropertyList(disabled<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };
}
