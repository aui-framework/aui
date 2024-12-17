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
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {
    /**
     * @brief Controls the expanding of AView.
     * @ingroup ass
     */
    struct Expanding {
        unset_wrap<unsigned> expandingX;
        unset_wrap<unsigned> expandingY;

        Expanding() : expandingX(2), expandingY(2) {}
        Expanding(const unset_wrap<unsigned>& expandingX, const unset_wrap<unsigned>& expandingY) : expandingX(expandingX), expandingY(expandingY) {}
        Expanding(const unset_wrap<unsigned>& expanding) : expandingX(expanding), expandingY(expanding) {}
    };


    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Expanding>: IPropertyBase {
        private:
            Expanding mInfo;

        public:
            Property(const Expanding& info) : mInfo(info) {
            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}