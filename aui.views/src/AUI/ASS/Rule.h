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

#include <AUI/ASS/Selector/AAssSelector.h>

namespace ass {
    struct Rule : PropertyList {
    public:
        template<typename... Declarations>
        Rule(ass::AAssSelector selector, Declarations&& ... declarations):
                PropertyList(std::forward<Declarations>(declarations)...),
                mSelector(std::move(selector)) {
        }

        [[nodiscard]] const ass::AAssSelector& getSelector() const {
            return mSelector;
        }

    private:
        ass::AAssSelector mSelector;
    };
}