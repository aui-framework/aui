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

#pragma once

#include <variant>
#include <AUI/Util/AMetric.h>

namespace ass {

/**
 * @brief Rectangular (sharp-cornered) shape.
 * @ingroup ass
 */
struct AShapeRectangle {};

/**
 * @brief Rounded rectangle shape with a uniform corner radius.
 * @ingroup ass
 */
struct AShapeRoundedRectangle {
    AMetric radius;
};

/**
 * @brief Describes the geometric shape used when drawing backgrounds and borders.
 * @ingroup ass
 *
 * @details
 * Currently AShape is a simple std::variant of concrete shape structs.
 *
 * **Planned redesign** (inspired by Jetpack Compose `Shape` / `Outline`):
 *   AShape will become an interface (or abstract base via std::shared_ptr) that exposes a
 *   `createOutline(glm::ivec2 size, ATextDirection textDirection = ATextDirection::LTR) -> AOutline` (or similar) factory method.
 *   `AShapeRectangle` and `AShapeRoundedRectangle` will be concrete implementations.
 *   This will allow user-defined shapes (e.g. cut-corner, ticket, star) without modifying
 *   the framework.
 */
using AShape = std::variant<AShapeRectangle, AShapeRoundedRectangle>;

} // namespace ass
