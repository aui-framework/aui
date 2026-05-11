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

//
// Created by Nelonn on 5/6/2026.
//

#pragma once

#include <compare>
#include <glm/glm.hpp>

/**
 * Layout constraints for a UI element.
 *
 * Defines minimum and maximum allowed inline and block sizes.
 * A maximum value of -1 means that the size is unlimited.
 *
 * The constraint is considered tight when its minimum and maximum
 * values are equal.
 */
struct AConstraints {
  int minInline = 0;
  int maxInline = -1;
  int minBlock = 0;
  int maxBlock = -1;

  static AConstraints fixedInline(int value) {
    return AConstraints {
      .minInline = value,
      .maxInline = value,
    };
  }

  static AConstraints fixedBlock(int value) {
    return AConstraints {
      .minBlock = value,
      .maxBlock = value,
    };
  }

  constexpr bool isInlineTight() const noexcept {
    return minInline == maxInline;
  }

  constexpr bool isBlockTight() const noexcept {
    return minBlock == maxBlock;
  }

  constexpr bool isUnlimitedInline() const noexcept {
    return maxInline == -1;
  }

  constexpr bool isUnlimitedBlock() const noexcept {
    return maxBlock == -1;
  }

  constexpr glm::ivec2 min() const noexcept {
    return {minInline, minBlock};
  }

  constexpr glm::ivec2 max() const noexcept {
    return {maxInline, maxBlock};
  }

  constexpr auto operator<=>(const AConstraints&) const = default;
  constexpr bool operator==(const AConstraints&) const = default;
};
