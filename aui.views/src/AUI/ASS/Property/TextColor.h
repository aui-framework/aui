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
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls the text color of AView.
     * @ingroup ass_properties
     */
    struct TextColor {
        std::variant<AColor, inherit_t> color;
    };


    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<TextColor>: IPropertyBase {
        private:
            TextColor mInfo;

        public:
            Property(const TextColor& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}