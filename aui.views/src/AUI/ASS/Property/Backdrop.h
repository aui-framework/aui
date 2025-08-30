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

#include <initializer_list>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents backdrop filter effect which applied to the pixels behind the view (i.e., blur, grayscale, etc.)
     * @ingroup ass_properties
     * @details
     * Matches CSS property `backdrop-filter`.
     *
     * Because the effect is applied to everything behind the view, to see the effect the view's background needs to be
     * transparent or partially transparent.
     */
    struct Backdrop {
        /**
         * @brief Underlying type of GaussianBlur but with customizable downscale. Generally, use GaussianBlur.
         */
        struct GaussianBlurCustom {
            /**
             * @brief blur radius.
             * @details
             * Performance costs of radius is O^2. Please use downscale factor to approximate large blur radius.
             */
            AMetric radius = 6_dp;

            /**
             * @brief downscale factor. =1 equals don't affect. Must be positive.
             * @details
             * Effective blur radius is `radius * downscale`. However, the downscale part is done by cheap downscaling
             * of the framebuffer texture. Thus, by raising downscale factor you can achieve larger blur radius with
             * approximately same visual result.
             *
             * Performance benefit of downscale factor { .radius = 6_dp, .downscale = x } in comparison to
             * { .radius = 6_dp * x, .downscale = 1 } is x^2.
             *
             * ```cpp
             * view AUI_WITH_STYLE {
             *   Backdrop { Backdrop::GaussianBlur { .radius = 6_dp, .downscale = 4 } },
             * }
             * ```
             * is visually approximately same as
             * ```cpp
             * view AUI_WITH_STYLE {
             *   Backdrop { Backdrop::GaussianBlur { .radius = 24_dp } },
             * }
             * ```
             * but 16 times cheaper.
             */
            int downscale = 1;


            [[nodiscard]]
            bool operator==(const GaussianBlurCustom&) const = default;
        };

        /**
         * @brief Fast gaussian blur.
         * @details
         * Strictly speaking, it's not an actual gaussian blur but a faster implementation of it.
         *
         * GaussianBlur calculates best downscale parameter applicable for passed radius.
         *
         * Downscale factor allows to produce approximately same visual result at drastically lower cost. You can use
         * GaussianBlurCustom directly to specify your downscale factor.
         */
        struct GaussianBlur {
            AMetric radius;

            [[nodiscard]]
            API_AUI_VIEWS GaussianBlurCustom findOptimalParams() const;
        };

        struct LiquidFluid {
        };

        using Any = std::variant<GaussianBlur, GaussianBlurCustom, LiquidFluid>;
        using Preprocessed = std::variant<GaussianBlurCustom, LiquidFluid>;
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