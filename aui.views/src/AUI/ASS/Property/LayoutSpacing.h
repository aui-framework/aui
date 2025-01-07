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


#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls the gap between elements of the container. Basically acts like a margin, but the gaps appear
     * between views only, not around them.
     *
     * @ingroup ass_properties
     */
    struct LayoutSpacing {
        AMetric spacing;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<LayoutSpacing>: IPropertyBase {
        private:
            LayoutSpacing mInfo;

        public:
            Property(const LayoutSpacing& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}