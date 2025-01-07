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
#include "AUI/Common/IStringable.h"
#include "IProperty.h"

namespace ass {


    /**
     * @brief Controls the max size of AView.
     * @ingroup ass_properties
     */
    struct MaxSize {
        unset_wrap<AMetric> width;
        unset_wrap<AMetric> height;

        MaxSize(const unset_wrap <AMetric>& width, const unset_wrap <AMetric>& height) : width(width), height(height) {}
        explicit MaxSize(const AMetric& side) : width(side), height(side) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<MaxSize>: IPropertyBase, IStringable {
        private:
            MaxSize mInfo;

        public:
            Property(const MaxSize& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

            AString toString() const override {
                return "MaxSize({}, {})"_format(mInfo.width, mInfo.height);
            }
        };
    }
}