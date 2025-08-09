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
        template<typename T>
        struct Type : IAssSubSelector {
            static_assert(std::is_polymorphic_v<T>, "Your type is not polymorphic! Please define at least virtual "
                                                    "destructor for using type_of (t) style selector "
                                                    "(see https://en.cppreference.com/w/cpp/language/object#Polymorphic_objects)");
        public:
            bool isPossiblyApplicable(AView* view) override {
                return dynamic_cast<T*>(view) != nullptr;
            }

        };
    }

    template<typename T>
    struct type_of: detail::Type<T> {
    public:

        using hover = ass::hovered<detail::Type<T>>;
        using active = ass::activated<detail::Type<T>>;
        using focus = ass::activated<detail::Type<T>>;
        using disabled = ass::disabled<detail::Type<T>>;
    };

    /**
     * @brief Selects views that are of the specified C++ types.
     * @ingroup ass_selectors
     * @details
     * This selector selects views that are of the specified classes. The selector can be accessed explicitly via
     * `ass::type_of`.
     *
     * For example:
     * ```cpp
     * {
     *   t<AButton>(),
     *   BackgroundSolid(...),
     * }
     * ```
     * This will select all `AButton`s.
     *
     */
    template<typename T>
    using t = type_of<T>;
}