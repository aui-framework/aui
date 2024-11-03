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

#include "IProperty.h"

namespace ass {

    /**
     * @brief Specified the stack order of the view.
     * @ingroup ass
     * @details
     * A view with greater stack order is always in front of a view with lower stack order value.
     *
     * If two views overlap with z-index value, the latter will be shown on top.
     */
    struct ZIndex {
        int value = 0;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<ZIndex>: IPropertyBase {
        private:
            ZIndex mInfo;

        public:
            Property(ZIndex info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}
