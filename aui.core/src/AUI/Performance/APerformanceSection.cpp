// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include "APerformanceSection.h"

#include <chrono>
#include <functional>
#include <random>
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Performance/APerformanceFrame.h"

#if AUI_PROFILING

using namespace std::chrono;

APerformanceSection::APerformanceSection(const char* name, AOptional<AColor> color)
    : mName(name),
      mColor(color.valueOr([&] { return generateColorFromName(mName); })),
      mStart(high_resolution_clock::now()) {}

APerformanceSection::~APerformanceSection() {
    auto delta = high_resolution_clock::now() - mStart;

    AUI_NULLSAFE(APerformanceFrame::current())->addSection({
        .name = std::move(mName),
        .color = mColor,
        .duration = delta,
    });
}

AColor APerformanceSection::generateColorFromName(const AString& name) {
    std::uint64_t seed = 0;
    for (auto c : name) {
        seed ^= c;
        seed <<= 1;
    }

    std::default_random_engine re(seed);

    std::uniform_real_distribution d(0.f, 1.f);

    return { d(re), d(re), d(re), 1.f };
}

#endif