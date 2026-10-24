/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <limits>

/**
 * Platform-independent window geometry math. Native window systems that measure in points (macOS) convert from/to real
 * pixels with the backing scale factor of the screen; this is NOT AUI's dpi ratio, which additionally includes
 * ASurface::ScalingParams and only affects dp<->px conversion.
 */
namespace aui::platform::detail {

/**
 * @brief Value of a native size meaning "no limit".
 */
constexpr double UNLIMITED_POINTS = std::numeric_limits<double>::max();

[[nodiscard]]
constexpr int pointsToPixels(double points, double backingScale) noexcept {
    return static_cast<int>(points * backingScale);
}

[[nodiscard]]
constexpr double pixelsToPoints(int pixels, double backingScale) noexcept {
    return pixels / backingScale;
}

/**
 * @brief Converts a max size dimension in pixels to points. Negative value means "unlimited" (see AView::getMaxSize)
 * and must stay unlimited: a negative size makes native window systems clamp the window to zero.
 */
[[nodiscard]]
constexpr double maxPixelsToPoints(int pixels, double backingScale) noexcept {
    return pixels < 0 ? UNLIMITED_POINTS : pixelsToPoints(pixels, backingScale);
}

}   // namespace aui::platform::detail
