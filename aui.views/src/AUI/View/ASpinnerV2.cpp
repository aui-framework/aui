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

#include "ASpinnerV2.h"

#include <AUI/Render/IRenderer.h>

using namespace std::chrono;
using namespace std::chrono_literals;

ASpinnerV2::ASpinnerV2() {}

void ASpinnerV2::render(ARenderContext ctx) {
    if (!mTimer) {
        AView::render(ctx);
        return;
    }
    ctx.render.translate(glm::vec2(getSize()) / 2.f);
    const auto now = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    ctx.render.rotate(
        AAngleRadians(float((now % mConfiguration.period) * mConfiguration.steps / mConfiguration.period) / float(mConfiguration.steps) * 2.f * glm::pi<float>()));
    ctx.render.translate(-glm::vec2(getSize()) / 2.f);
    AView::render(ctx);
}

void ASpinnerV2::setConfiguration(ASpinnerV2::Configuration configuration) {
    mConfiguration = std::move(configuration);
    if (mConfiguration.period < 1ms || mConfiguration.steps == 0) {
        mTimer = nullptr;
        return;
    }
    mTimer = _new<ATimer>(mConfiguration.period / mConfiguration.steps);
    connect(mTimer->fired, me::redraw);
    mTimer->start();
}

void ass::prop::Property<ASpinnerV2::Configuration>::applyFor(AView *view) {
    if (auto s = dynamic_cast<ASpinnerV2 *>(view)) {
        s->setConfiguration(mConfiguration);
    }
}
