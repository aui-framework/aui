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

#include "GaussianKernel.h"
#include "AUI/Common/AMap.h"

static float gaussian(float x, float mu, float sigma) {
    const float a = (x - mu) / sigma;
    return glm::exp(-0.5f * a * a);
}

AArrayView<float> aui::detail::gaussianKernel(unsigned int radius) {
    using Kernel = AVector<float>;
    static AUnorderedMap<unsigned /* radius */, Kernel> kernels;
    return kernels.getOrInsert(radius, [radius]() {
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
}
