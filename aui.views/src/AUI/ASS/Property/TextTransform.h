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
// Created by alex2 on 07.01.2021.
//


#pragma once

#include <AUI/Enum/TextTransform.h> // defined here
#include "IProperty.h"
#include <AUI/Util/Declarative/Modifier.h>

namespace ass::legacy {
    template<>
    struct API_AUI_VIEWS Property<TextTransform>: IPropertyBase {
    private:
        TextTransform mInfo;

    public:
        Property(const TextTransform& info) : mInfo(info) {

        }

        void applyFor(AView* view) override;

        [[nodiscard]]
        const auto& value() const noexcept {
            return mInfo;
        }
    };

    API_AUI_VIEWS Modifier operator|(Modifier thiz, TextTransform value);
}