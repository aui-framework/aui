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

#include "IRendererBackend.h"
#include <range/v3/all.hpp>
#include <AUI/Traits/callables.h>

void IRendererBackend::backdrops(const ADisplayList::Backdrop& v, const glm::mat4& transform) {
    auto p1 = glm::vec2(transform * glm::vec4(v.position, 0.f, 1.f));
    auto p2 = glm::vec2(transform * glm::vec4(v.position + v.size, 0.f, 1.f));
    auto lower = glm::floor(glm::min(p1, p2));
    auto upper = glm::ceil(glm::max(p1, p2));

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
    this->backdrops(glm::ivec2(lower), glm::ivec2(upper - lower),
                    std::span<const Preprocessed>(preprocessed.data(), preprocessed.size()));
}
