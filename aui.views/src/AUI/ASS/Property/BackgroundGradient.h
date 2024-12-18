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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include "IProperty.h"

namespace ass {
    /**
     * @brief Represents gradient background.
     * @ingroup ass
     */
    struct BackgroundGradient {
        AOptional<ALinearGradientBrush> gradient;

        BackgroundGradient(std::nullptr_t) noexcept {};
        BackgroundGradient(ALinearGradientBrush brush) noexcept: gradient(std::move(brush)) {}

        BackgroundGradient(AColor begin, AColor end, AAngleRadians angle) noexcept: gradient(ALinearGradientBrush{
                .colors = {
                        {0.f, begin},
                        {1.f, end},
                },
                .rotation = angle
        }) {}
        BackgroundGradient(AColor begin, AColor end, ALayoutDirection direction) noexcept: BackgroundGradient(begin, end, direction == ALayoutDirection::VERTICAL ? 180_deg : 90_deg) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundGradient>: IPropertyBase {
        private:
            BackgroundGradient mInfo;

        public:
            Property(const BackgroundGradient& info) : mInfo(info) {

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