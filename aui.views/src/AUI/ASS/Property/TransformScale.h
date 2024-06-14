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
// Created by alex2 on 07.01.2021.
//


#pragma once


#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls the rendering scale transform of AView.
     * @ingroup ass
     */
    struct TransformScale {
        glm::vec2 scale;

        TransformScale(float scaleX, float scaleY) : scale(scaleX, scaleY) {}
        explicit TransformScale(float scale) : scale(scale) {}
    };


    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<TransformScale>: IPropertyBase {
        private:
            TransformScale mInfo;

        public:
            Property(const TransformScale& info) : mInfo(info) {}
            void renderFor(AView* view) override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}