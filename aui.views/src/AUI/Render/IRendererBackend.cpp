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
