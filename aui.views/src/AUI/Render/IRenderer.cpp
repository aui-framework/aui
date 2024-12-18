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

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/algorithm/transform.hpp>

#include "IRenderer.h"

void IRenderer::stub(glm::vec2 position, glm::vec2 size) {
    rectangle(ASolidBrush{0xa0a0a0_rgb}, {0, 0}, size);
}

static float gaussian(float x, float mu, float sigma) {
    const float a = ( x - mu ) / sigma;
    return glm::exp(-0.5f * a * a);
}

void IRenderer::blurGaussian(glm::vec2 position, glm::vec2 size, int radius, int downscale) {
    using Kernel = AVector<float>;
    static AUnorderedMap<int /* radius */, Kernel> kernels;
    const auto& kernel = kernels.getOrInsert(radius, [radius]() {
      const float sigma = float(radius) / 2.f;

      // generate values by gaussian function
      auto result = AVector<float>::generate(radius * 2 + 1, [&](int i) {
        return gaussian(float(i), float(radius), sigma);
      });

      // normalize result
      auto sum = ranges::accumulate(result, 0.f);
      ranges::transform(result, result.begin(), [&](float i) { return i / sum; });
      return result;
    });
    blur(position, size, radius, downscale, kernel);
}

void IRenderer::blur(glm::vec2 position, glm::vec2 size, int radius, int downscale, AArrayView<float> kernel) {
    stub(position, size);
}
