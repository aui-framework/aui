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

#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Performance/APerformanceFrame.h"
#include <chrono>
#include <functional>
#include <random>

#if AUI_PROFILING

using namespace std::chrono;
using namespace std::chrono_literals;

static constexpr auto THRESHOLD = 50us;

APerformanceSection::APerformanceSection(const char* name, AOptional<AColor> color, std::string verboseInfo)
    : mName(name),
      mColor(color.valueOr([&] { return generateColorFromName(mName); })),
      mVerboseInfo(std::move(verboseInfo)),
      mStart(high_resolution_clock::now()), mParent(current()) {
  if (!APerformanceFrame::current()) {
    return;
  }
  current() = this;
}

APerformanceSection::~APerformanceSection() {
  if (!APerformanceFrame::current()) {
    return;
  }

  current() = mParent;

  auto delta = high_resolution_clock::now() - mStart;
  if (delta < THRESHOLD) {
    return;
  }

  std::variant<APerformanceSection*, APerformanceFrame*> sectionReceiever =
      APerformanceFrame::current();

  if (mParent) {
    sectionReceiever = mParent;
  }

  std::visit(
      [&](auto value) {
        if (!value) {
          return;
        }
        value->addSection({
            .name = mName,
            .color = mColor,
            .verboseInfo = std::move(mVerboseInfo),
            .duration = delta,
            .children = std::move(mChildren),
        });
      },
      sectionReceiever);
}

AColor APerformanceSection::generateColorFromName(const char* name) {
  std::uint64_t seed = 0;
  for (auto c : std::string_view(name)) {
    seed ^= c;
    seed <<= 1;
  }

  std::default_random_engine re(seed);

  std::uniform_real_distribution d(0.f, 1.f);

  return {d(re), d(re), d(re), 1.f};
}

#endif