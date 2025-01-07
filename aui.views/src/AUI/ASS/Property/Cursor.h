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

#include <AUI/Platform/ACursor.h> // defined here
#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {
    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<ACursor>: IPropertyBase {
        private:
            ACursor mInfo;

        public:
            Property(const ACursor& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
        template<>
        struct API_AUI_VIEWS Property<ACursor::System>: Property<ACursor> {
            using Property<ACursor>::Property;
        };
    }
}
