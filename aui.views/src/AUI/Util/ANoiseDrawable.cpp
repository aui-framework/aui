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

#include "ANoiseDrawable.h"
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/IRendererBackend.h>
#include <AUI/Util/ARandom.h>

void ANoiseDrawable::draw(ARenderContext ctx, const IDrawable::Params& params) {
    if (!mNoise) {
        auto& backend = ctx.backend;
        mNoise = backend.createTexture({ 32, 32 }, APixelFormat::R8G8B8A8_UNORM);

        AFormattedImage<APixelFormat::R8G8B8A8_UNORM> data({ 32, 32 });
        ARandom r;
        for (auto& color : data) {
            color.r = color.g = color.b = uint8_t(r.nextInt());
            color.a = 255;
        }
        mNoise->upload(AImageView(data));
        }

    ctx.canvas.rectangle(APaint {
        .brush = ATexturedBrush {
            .texture = mNoise,
            .uv1 = glm::vec2(0),
            .uv2 = glm::vec2(params.size) * glm::vec2(0.37390528174893522421f, 0.37577434667f),
            .imageRendering = ImageRendering::SMOOTH,
            .repeat = Repeat::X_Y,
        },
        .color = params.color,
    }, params.offset, params.size);
}

glm::ivec2 ANoiseDrawable::getSizeHint() { return {1, 1}; }
