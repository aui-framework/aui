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

#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Util/Declarative.h>

/**
 * @brief Fixed-size view which is useful in UI building.
 * @ingroup views_arrangement
 * @details
 *
 * ASpacerFixed is fixed size blank view which acquires specified space in `Horizontal` and `Vertical` layouts. For the
 * rest of layout managers it does nothing.
 *
 * ASpacerFixed follows direction of its container layout. That is, for `Horizontal` layout it consumes horizontal space
 * only and for `Vertical` layout it consumes vertical space only.
 *
 * Example usage:
 * ```cpp
 * Horizontal {
 *   Label { "Hello" },
 *   SpacerFixed(100_dp),
 *   Label { "world" },
 * }
 * ```
 * In this case, SpacerFixed occupies `100_dp` in horizontal but nothing in vertical.
 */
class API_AUI_VIEWS ASpacerFixed : public AView {
public:
    explicit ASpacerFixed(AMetric space) : mSpace(space) {}
    bool consumesClick(const glm::ivec2& pos) override;
    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;

private:
    AMetric mSpace;
};

namespace declarative {

/**
 * @declarativeformof{ASpacerFixed}
 */
struct SpacerFixed : aui::ui_building::view<ASpacerFixed> {
    using aui::ui_building::view<ASpacerFixed>::view;
};
}   // namespace declarative