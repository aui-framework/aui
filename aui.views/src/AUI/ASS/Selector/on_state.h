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

    /**
     * @brief Hovered LESS-style subselector.
     * @ingroup ass_selectors
     * @details
     * It's convenient to use it with with_style:
     * @code{cpp}
     * _new<AView>() with_style {
     *   MinSize { 16_dp },
     *   BackgroundSolid { AColor::BLACK },
     *   on_state::Hovered {
     *     BackgroundSolid { AColor::RED },
     *   },
     * },
     * @endcode
     */
    struct Hovered: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Hovered(Declarations&&... declarations):
                ConditionalPropertyList(hovered<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    /**
     * @brief Activated LESS-style subselector.
     * @ingroup ass_selectors
     * @details
     * It's convenient to use it with with_style:
     * @code{cpp}
     * _new<AView>() with_style {
     *   MinSize { 16_dp },
     *   BackgroundSolid { AColor::BLACK },
     *   on_state::Activated {
     *     BackgroundSolid { AColor::RED },
     *   },
     * },
     * @endcode
     */
    struct Activated: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Activated(Declarations&&... declarations):
                ConditionalPropertyList(activated<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    /**
     * @brief Focused LESS-style subselector.
     * @ingroup ass_selectors
     * @details
     * It's convenient to use it with with_style:
     * @code{cpp}
     * _new<AView>() with_style {
     *   MinSize { 16_dp },
     *   BackgroundSolid { AColor::BLACK },
     *   on_state::Focused {
     *     BackgroundSolid { AColor::RED },
     *   },
     * },
     * @endcode
     */
    struct Focused: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Focused(Declarations&&... declarations):
                ConditionalPropertyList(focused<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };

    /**
     * @brief Disabled LESS-style subselector.
     * @ingroup ass_selectors
     * @details
     * It's convenient to use it with with_style:
     * @code{cpp}
     * _new<AView>() with_style {
     *   MinSize { 16_dp },
     *   BackgroundSolid { AColor::BLACK },
     *   on_state::Disabled {
     *     BackgroundSolid { AColor::RED },
     *   },
     * },
     * @endcode
     */
    struct Disabled: public PropertyListRecursive::ConditionalPropertyList {
    public:
        template<typename... Declarations>
        Disabled(Declarations&&... declarations):
                ConditionalPropertyList(disabled<impl::OnStateSelector>{}, std::forward<Declarations>(declarations)...)
        {}
    };
}
