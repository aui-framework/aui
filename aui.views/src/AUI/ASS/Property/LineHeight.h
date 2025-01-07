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
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {
    /**
     * @brief Controls line height multiplier of the font of the AView.
     * @ingroup ass_properties
     * @details
     * Matches CSS property line-height.
     */
    struct LineHeight {
        float spacing;

        /**
         * @brief LineHeight matching 'line-height: normal' on common browsers (= 1.2). Used by default.
         */
        static LineHeight NORMAL;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<LineHeight>: IPropertyBase {
        private:
            LineHeight mInfo;

        public:
            Property(const LineHeight& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}

inline ass::LineHeight ass::LineHeight::NORMAL = { 1.2f };
