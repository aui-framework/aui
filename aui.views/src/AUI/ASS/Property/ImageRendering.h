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

#include <AUI/Util/ALayoutDirection.h>
#include "IProperty.h"
#include <AUI/Enum/ImageRendering.h>

namespace ass {

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<ImageRendering>: IPropertyBase {
        private:
            ImageRendering mInfo;

        public:
            Property(const ImageRendering& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;
            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };

    }
}