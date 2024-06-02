// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Alex2772 on 11/19/2021.
//

#include <range/v3/algorithm.hpp>
#include <range/v3/numeric.hpp>


#include <AUI/Util/ACleanup.h>
#include <AUI/Util/kAUI.h>
#include "ARender.h"


_<IRenderer> ARender::ourRenderer;

void ARender::setRenderer(_<IRenderer> renderer) {
    ourRenderer = std::move(renderer);
    do_once ACleanup::afterEntry([] {
        ourRenderer = nullptr;
    });
}

static float gaussian(float x, float mu, float sigma) {
    const float a = ( x - mu ) / sigma;
    return glm::exp(-0.5f * a * a);
}

void ARender::blur(glm::vec2 position, glm::vec2 size, int radius) {
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
    ourRenderer->drawBlur(position, size, radius, kernel);
}
