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

#include <initializer_list>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents backdrop filter effect which applied to the pixels behind the view (i.e., blur, grayscale, etc.)
     * @ingroup ass
     * @details
     * Matches CSS property `backdrop-filter`.
     *
     * Because the effect is applied to everything behind the view, to see the effect the view's background needs to be
     * transparent or partially transparent.
     */
    struct Backdrop {
        struct GaussianBlur {
            /*
             * @brief blur radius. Must be odd number
             */
            int radius = 13;

            /*
             * @brief downscale factor. =1 equals don't affect
             */
            int downscale = 1;
        };

        using Any = std::variant<GaussianBlur>;
        AVector<Any> effects;

        Backdrop(std::initializer_list<Any> effects): effects(std::move(effects)) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Backdrop>: IPropertyBase {
        private:
            Backdrop mInfo;

        public:
            Property(const Backdrop& info) : mInfo(info) {

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