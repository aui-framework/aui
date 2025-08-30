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
     * @brief Controls the padding of AView.
     * @ingroup ass_properties
     * @details
     * See [aui-box-model].
     */
    struct Padding {
        unset_wrap<AMetric> left;
        unset_wrap<AMetric> top;
        unset_wrap<AMetric> right;
        unset_wrap<AMetric> bottom;

        Padding(unset_wrap<AMetric> all):
            left(all),
            top(all),
            right(all),
            bottom(all)
        {
        }
        Padding(unset_wrap<AMetric> vertical, unset_wrap<AMetric> horizontal):
                left(horizontal),
                top(vertical),
                right(horizontal),
                bottom(vertical)
        {
        }
        Padding(unset_wrap<AMetric> top, unset_wrap<AMetric> horizontal, unset_wrap<AMetric> bottom):
                left(horizontal),
                top(top),
                right(horizontal),
                bottom(bottom)
        {
        }
        Padding(unset_wrap<AMetric> top, unset_wrap<AMetric> right, unset_wrap<AMetric> bottom, unset_wrap<AMetric> left):
                left(left),
                top(top),
                right(right),
                bottom(bottom)
        {
        }
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Padding>: IPropertyBase {
        private:
            Padding mInfo;

        public:
            Property(const Padding& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}