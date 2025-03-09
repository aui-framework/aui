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
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Custom property with user-defined handlers.
     * @ingroup ass_properties
     */
    struct CustomProperty {
        std::function<void(AView*)> onApplyFor;
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<CustomProperty>: IPropertyBase {
        private:
            CustomProperty mInfo;

        public:
            Property(CustomProperty info) : mInfo(std::move(info)) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}