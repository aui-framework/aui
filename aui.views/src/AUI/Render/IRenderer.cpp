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

#include <range/v3/all.hpp>
#include "IRenderer.h"

void IRenderer::stub(glm::vec2 position, glm::vec2 size) {
    rectangle(ADisplayList::Rectangle{{ 0, 0 }, size}, APaint{ASolidBrush{0xa0a0a0_rgb}});
}

void IRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<const ass::Backdrop::Preprocessed> backdrops) {
    stub(position, size);
}

void IRenderer::backdrops(const ADisplayList::Backdrop& v, const APaint& paint) {
    using Preprocessed = ass::Backdrop::Preprocessed;
    auto preprocessed =
        v.backdrops | ranges::views::transform([](const ass::Backdrop::Any& val) -> Preprocessed {
            return std::visit(
                aui::lambda_overloaded {
                  [](const ass::Backdrop::GaussianBlur& b) -> Preprocessed {
                      return b.findOptimalParams();
                  },
                  [](const auto& b) -> Preprocessed { return b; },
                },
                val);
        }) |
        ranges::to_vector;
    this->backdrops(v.position, v.size, std::span<const Preprocessed>(preprocessed.data(), preprocessed.size()));
}
