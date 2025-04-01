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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents solid (single color) background.
     * @ingroup ass_properties
     */
    struct BackgroundSolid {
        AColor color;

        BackgroundSolid(const AColor& color) : color(color) {}
        BackgroundSolid(std::nullptr_t) : color(0.f) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundSolid>: IPropertyBase {
        private:
            BackgroundSolid mInfo;

        public:
            Property(const BackgroundSolid& info) : mInfo(info) {

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