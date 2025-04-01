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
// Created by dervisdev on 1/26/2023.
//

#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IProperty.h"



namespace ass {

    /**
     * @brief Controls how do scrollbars and content appear in AScrollArea. This rule is applicable to AScrollArea only.
     * @ingroup ass_properties
     */
    struct ScrollbarAppearance {
    public:
        /**
         * @brief Per-axis behaviour enum.
         */
        enum AxisValue {
            /**
             * @brief Scrollbar is always visible, no matter whether or not any content is overflowing.
             */
            ALWAYS,

            /**
             * @brief Scrollbar appears only if content is overflowing.
             */
            ON_DEMAND,

            /**
             * @brief Scrollbar is always gone, no matter whether or not any content is overflowing.
             */
            NEVER,
        };

        AxisValue getVertical() {
            return mVertical;
        }

        AxisValue getHorizontal() {
            return mHorizontal;
        }

        ScrollbarAppearance() = default;
        explicit ScrollbarAppearance(AxisValue both) : mVertical(both), mHorizontal(both) {};
        ScrollbarAppearance(AxisValue vertical, AxisValue horizontal) : mVertical(vertical), mHorizontal(horizontal) {};

    private:
        AxisValue mVertical = AxisValue::ON_DEMAND;
        AxisValue mHorizontal = AxisValue::ON_DEMAND;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<ScrollbarAppearance>: IPropertyBase {
        private:
            ScrollbarAppearance mInfo;

        public:
            Property(const ScrollbarAppearance& info) : mInfo(info) { }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}