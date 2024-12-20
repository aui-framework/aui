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

#include <range/v3/all.hpp>
#include "IRenderer.h"

void IRenderer::stub(glm::vec2 position, glm::vec2 size) { rectangle(ASolidBrush { 0xa0a0a0_rgb }, { 0, 0 }, size); }

void IRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Preprocessed> backdrops) {
    stub(position, size);
}
void IRenderer::backdrops(glm::ivec2 position, glm::ivec2 size, std::span<ass::Backdrop::Any> backdrops) {
    using Preprocessed = ass::Backdrop::Preprocessed;
    auto preprocessed =
        backdrops | ranges::view::transform([](const ass::Backdrop::Any& v) -> Preprocessed {
            return std::visit(
                aui::lambda_overloaded {
                  [](const ass::Backdrop::GaussianBlur& v) -> Preprocessed {
                      return v.findOptimalParams();
                  },
                  [](const auto& v) -> Preprocessed { return v; },
                },
                v);
        }) |
        ranges::to_vector;
    this->backdrops(position, size, std::span<Preprocessed>(preprocessed));
}
