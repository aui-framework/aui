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
#include "AUI/Performance/APerformanceFrame.h"

#if AUI_PROFILING

using namespace std::chrono;

APerformanceSection::APerformanceSection(const char* name, AOptional<AColor> color)
    : mName(name),
      mColor(color.valueOr([&] { return generateColorFromName(mName); })),
      mStart(high_resolution_clock::now()) {}

APerformanceSection::~APerformanceSection() {
    auto delta = high_resolution_clock::now() - mStart;

    APerformanceFrame::current().addSection({
        .name = std::move(mName),
        .color = mColor,
        .duration = delta,
    });
}

#endif