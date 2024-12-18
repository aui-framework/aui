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



#pragma once


#include <AUI/Util/AMetric.h>
#include <AUI/Traits/values.h>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Controls the opacity of AView.
     * @ingroup ass
     */
    struct Opacity {
        aui::float_within_0_1 opacity;

        explicit Opacity(float opacity) : opacity(opacity) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Opacity>: IPropertyBase {
        private:
            Opacity mInfo;

        public:
            Property(const Opacity& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}