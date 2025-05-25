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

#include "APerformanceSection.h"

#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Performance/APerformanceFrame.h"
#include <chrono>
#include <functional>
#include <random>

#if AUI_PROFILING

using namespace std::chrono;
using namespace std::chrono_literals;

static constexpr auto THRESHOLD = 3us;

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