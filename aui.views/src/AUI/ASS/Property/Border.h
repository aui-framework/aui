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
     * @brief Represents border.
     * @ingroup ass_properties
     * @details
     * See [aui-box-model].
     */
    struct Border {
        enum BorderType {
            SOLID,
        };

        AMetric width;
        BorderType type;
        AColor color;

        Border(const AMetric& width,
               BorderType type,
               const AColor& color):
            width(width),
            type(type),
            color(color) {

        }
        Border(const AMetric& width,
               const AColor& color):
            width(width),
            type(SOLID),
            color(color) {

        }
        Border(std::nullptr_t) {}

    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Border>: IPropertyBase {
        private:
            Border mInfo;

        public:
            Property(const Border& info) : mInfo(info) {

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