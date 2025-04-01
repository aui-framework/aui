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
// Created by alex2 on 29.12.2020.
//

#include <range/v3/all.hpp>
#include "Backdrop.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/IRenderer.h"
#include "IProperty.h"

void ass::prop::Property<ass::Backdrop>::renderFor(AView* view, const ARenderContext& ctx) {
    ctx.render.backdrops({}, view->getSize(), std::span<ass::Backdrop::Any>(mInfo.effects.data(), mInfo.effects.size()));
}

bool ass::prop::Property<ass::Backdrop>::isNone() { return mInfo.effects.empty(); }

ass::prop::PropertySlot ass::prop::Property<ass::Backdrop>::getPropertySlot() const {
    return ass::prop::PropertySlot::BACKDROP;
}
ass::Backdrop::GaussianBlurCustom ass::Backdrop::GaussianBlur::findOptimalParams() const {
    static constexpr auto MIN_OPTIMAL_DOWNSCALE = 1;
    static constexpr auto MAX_OPTIMAL_DOWNSCALE = 8; // larger values are too noisy
    static constexpr auto DOWNSCALE_DIVISOR = 8;
    auto originalRadiusPx = int(radius.getValuePx());

    auto downscale = glm::clamp(originalRadiusPx / DOWNSCALE_DIVISOR, MIN_OPTIMAL_DOWNSCALE, MAX_OPTIMAL_DOWNSCALE);
    return ass::Backdrop::GaussianBlurCustom {
        .radius = AMetric(float(originalRadiusPx) / downscale, AMetric::T_PX),
        .downscale = downscale,
    };
}
