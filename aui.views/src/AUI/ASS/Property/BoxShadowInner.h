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
     * @brief Represents box shadow.
     * @ingroup ass_properties
     * @details
     * Unlike CSS, box shadow is not affected by BorderRadius.
     */
    struct BoxShadowInner {
        AMetric offsetX;
        AMetric offsetY;
        AMetric blurRadius;
        AMetric spreadRadius;
        AColor color;

        BoxShadowInner(AMetric offsetX,
                  AMetric offsetY,
                  AMetric blurRadius,
                  AMetric spreadRadius,
                  const AColor& color):
            offsetX(offsetX),
            offsetY(offsetY),
            blurRadius(blurRadius),
            spreadRadius(spreadRadius),
            color(color) {}

        BoxShadowInner(AMetric offsetX,
                  AMetric offsetY,
                  AMetric blurRadius,
                  const AColor& color):
            offsetX(offsetX),
            offsetY(offsetY),
            blurRadius(blurRadius),
            color(color) {}

        BoxShadowInner(std::nullptr_t): color(0.f) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BoxShadowInner>: IPropertyBase {
        private:
            BoxShadowInner mInfo;

        public:
            Property(const BoxShadowInner& info) : mInfo(info) {

            }

            void renderFor(AView* view, const ARenderContext& ctx) override;

            bool isNone() override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}