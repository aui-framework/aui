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
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls the text shadow of AView.
     * @ingroup ass_properties
     */
    struct TextShadow {
        AColor shadowColor;
        AMetric offsetX = 1_dp;
        AMetric offsetY = 1_dp;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<TextShadow>: IPropertyBase {
        private:
            TextShadow mInfo;

        public:
            Property(const TextShadow& info) : mInfo(info) {

            }

            void renderFor(AView* view, const ARenderContext& ctx) override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}